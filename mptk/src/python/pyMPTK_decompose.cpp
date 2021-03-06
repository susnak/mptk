#include "pyMPTK.h"
#include "mpd_core.h"

extern PyTypeObject bookType;

MP_Signal_c* mp_create_signal_from_numpyarray(const PyArrayObject *nparray){
	unsigned long int nspls = nparray->dimensions[0];
	unsigned      int nchans;
	double *signal_data = (double *)nparray->data;
	const char *func = "mp_create_signal_from_numpyarray()";
	//printf("%s - numpy array has %d channels, %d samples\n", func, (int)nchans, (int)nspls);

	if(nparray->nd == 2){
		nchans = nparray->dimensions[1];
		if(nspls==1 && nchans>1){ // For user convenience, we can accommodate single-channel as row or as column
			nchans = 1;
			nspls  = nparray->dimensions[1];
		}
	} else if(nparray->nd == 1) {
		nchans = 1;
	} else {
		mp_error_msg(func,"input signal should be a numpy array (1D or 2D), of shape (numSamples, numChans)");
		return(NULL);
	}

	// Creating storage for output
	MP_Signal_c *signal = MP_Signal_c::init(nchans, nspls, 1);
	if (NULL==signal) {
		mp_error_msg(func, "Can't allocate a new signal.\n" );
		return(NULL);
	}

	// Copying content. NB I'm quite confident in-place operation is not a good idea, since mptk modifies the signal in-place
	for (unsigned int channel=0; channel < nchans; ++channel) {
		for (unsigned long int sample=0; sample < nspls; ++sample) {
			signal->channel[channel][sample] =  signal_data[channel*nspls +  sample];
		}
	}
	signal->refresh_energy();

	return(signal);
}


// Moved mptk_decompose into the main .cpp, since I think it needs to be in the same compiled object file in order not to crash on import_array() issues.
// The implementation of the main number-crunching calls goes here though.

int mptk_decompose_body(const PyArrayObject *numpysignal, const char *dictpath, const int samplerate, const unsigned long int numiters, const float snr, const char *method, const char* decaypath, const char* bookpath,
	unsigned long int reportEvery,
	unsigned long int cmpd_maxnum_cycles,
	double            cpmd_min_cycleimprovedb,
	unsigned long int cpmd_maxnum_aug_beforecycle,
	double            cpmd_maxnum_aug_beforecycle_db,
	unsigned long int cpmd_max_aud_stopcycle,
	double            cpmd_max_db_stopcycle,
	int cmpd_hold,
	mptk_decompose_result& result)
{
	// book, residual = mptk.decompose(sig, dictpath, samplerate, [ snr=0.5, numiters=10, method='mp', ... ])

	////////////////////////////////////////////////////////////
	// get signal in mem in appropriate format
	MP_Signal_c *signal = mp_create_signal_from_numpyarray(numpysignal);
	if(NULL==signal) {
		PyErr_SetString(PyExc_RuntimeError, "Failed to convert a signal from python to MPTK.\n");
		return 1;
	}
	signal->sampleRate = samplerate;

	////////////////////////////////////////////////////////////
	// get dict in mem in appropriate format - we'll do it from file - easy
	MP_Dict_c* dict = MP_Dict_c::read_from_xml_file(dictpath);
	if(NULL==dict) {
		PyErr_SetString(PyExc_RuntimeError, "Failed to read dict from file path.\n");
		delete signal;
		return 2;
	}

	////////////////////////////////////////////////////////////
	// Configure and run

	// Set up core and book obj
	MP_Book_c *mpbook = MP_Book_c::create(signal->numChans, signal->numSamples, signal->sampleRate );
	if ( NULL == mpbook )  {
	    PyErr_SetString(PyExc_RuntimeError, "Failed to create a book object.\n" );
	    delete signal;
	    delete dict;
	    return 3;
	}
	MP_Abstract_Core_c *mpdCore;
	if(strcmp(method, "")==0 || strcmp(method, "mp")==0){
		mpdCore =  MP_Mpd_Core_c::create( signal, mpbook, dict );
	}else if(strcmp(method, "cmp")==0){
		mpdCore =  MP_CMpd_Core_c::create( signal, mpbook, dict );
	//}else if(strcmp(method, "gmp")==0){
	//	mpdCore =  GPD_Core_c::create( signal, mpbook, dict );
	}else{
		PyErr_SetString(PyExc_RuntimeError, "Unrecognised 'method' option. Recognised options are: [mp, cmp]\n");
		delete signal;
		delete dict;
		delete mpbook;
		return 5;
	}
	if ( NULL == mpdCore )  {
	    PyErr_SetString(PyExc_RuntimeError, "Failed to create a MPD core object.\n" );
	    delete signal;
	    delete dict;
	    delete mpbook;
	    return 4;
	}

	// Set stopping condition
	if(numiters != 0){
		mpdCore->set_iter_condition( numiters );
	}else{
		mpdCore->set_snr_condition( snr );
	}
	if(strcmp(method, "")==0 || strcmp(method, "mp")==0){
		((MP_Mpd_Core_c*) mpdCore)->set_save_hit(ULONG_MAX, bookpath, NULL, decaypath);
	}else if(strcmp(method, "cmp")==0){
		((MP_CMpd_Core_c*) mpdCore)->set_save_hit(ULONG_MAX, bookpath, NULL, decaypath);
		((MP_CMpd_Core_c*) mpdCore)->set_settings( 		cmpd_maxnum_cycles, cpmd_min_cycleimprovedb, cpmd_maxnum_aug_beforecycle,
        (float) cpmd_maxnum_aug_beforecycle_db, cpmd_max_aud_stopcycle, (float)cpmd_max_db_stopcycle, cmpd_hold > 0 );

	}//else if(strcmp(method, "gmp")==0){
		// not same method... ((GPD_Core_c*) mpdCore)->set_save_hit(ULONG_MAX, bookpath, NULL, decaypath);
	//}
	mpdCore->set_report_hit(reportEvery);
	if(decaypath != NULL) mpdCore->set_use_decay();

	mpdCore->set_verbose();
	// Display some information
	//printf("The dictionary contains %d blocks\n",dict->numBlocks);
	//printf("The signal has:\n");
	//signal->info();
	//mpdCore->info_conditions();
	//printf("Initial signal energy is %g\n",mpdCore->get_initial_energy());
	//printf("Starting to iterate\n");

	mpdCore->run();
	mpdCore->info_state();
	mpdCore->info_result();


	// Get results - book, residual, decay. as a first pass, maybe we should write the book to disk, return the residual, ignore the decay (useDecay=false).

	// write book XML and/or decay data to disk.
	mpdCore->save_result();

	// create python book object, which will be returned
	BookObject* thebook = (BookObject*)PyObject_CallObject((PyObject *) &bookType, NULL);
	pybook_from_mpbook(thebook, mpbook);
	//Py_INCREF(thebook); // TODO this may rescue us from losing data, or it may be a memory leak

	result.thebook = thebook;
	result.residual = mp_create_numpyarray_from_signal(signal); // residual is in here (i.e. the "signal" is updated in-place)

	//printf("book stats: numChans %i, numSamples %il, sampleRate %il, numAtoms %i.\n", mpbook->numChans, mpbook->numSamples, mpbook->sampleRate, mpbook->numAtoms);

	delete signal;
	delete dict;
	//NO! delete mpbook;
	delete mpdCore;

	return 0;
}

