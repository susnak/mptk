/******************************************************************************/
/*                                                                            */
/*                              harmonic_atom.cpp                             */
/*                                                                            */
/*                        Matching Pursuit Library                            */
/*                                                                            */
/* R�mi Gribonval                                                             */
/* Sacha Krstulovic                                           Mon Feb 21 2005 */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/*  Copyright (C) 2005 IRISA                                                  */
/*                                                                            */
/*  This program is free software; you can redistribute it and/or             */
/*  modify it under the terms of the GNU General Public License               */
/*  as published by the Free Software Foundation; either version 2            */
/*  of the License, or (at your option) any later version.                    */
/*                                                                            */
/*  This program is distributed in the hope that it will be useful,           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*  GNU General Public License for more details.                              */
/*                                                                            */
/*  You should have received a copy of the GNU General Public License         */
/*  along with this program; if not, write to the Free Software               */
/*  Foundation, Inc., 59 Temple Place - Suite 330,                            */
/*  Boston, MA  02111-1307, USA.                                              */
/*                                                                            */
/******************************************************************************/


/*************************************************************/
/*                                                           */
/* harmonic_atom.cpp: methods for Harmonic Gabor atoms       */
/*                                                           */
/*************************************************************/

#include "mptk.h"
#include "gabor_atom_plugin.h"
#include "harmonic_atom_plugin.h"
#include "mp_system.h"

#include <dsp_windows.h>

using namespace std;





/***************************/
/* CONSTRUCTORS/DESTRUCTOR */
/***************************/

/*****************************/
/* factory function empty */

MP_Atom_c* MP_Harmonic_Atom_Plugin_c::harmonic_atom_create_empty (MP_Dict_c* dict)
{
  return new MP_Harmonic_Atom_Plugin_c(dict);
}

/*************************/
/* File factory function */
MP_Atom_c* MP_Harmonic_Atom_Plugin_c::create_fromxml( TiXmlElement *xmlobj, MP_Dict_c *dict)
{
  const char* func = "MP_Harmonic_Atom_c::create_fromxml(fid,mode)";
  MP_Harmonic_Atom_Plugin_c* newAtom = NULL;

  /* Instantiate and check */
  newAtom = new MP_Harmonic_Atom_Plugin_c(dict);
  if ( newAtom == NULL )
    {
      mp_error_msg( func, "Failed to create a new atom.\n" );
      return( NULL );
    }

	// Read and check
	if ( newAtom->init_fromxml( xmlobj ) )
	{
		mp_error_msg( func, "Failed to read the new Harmonic atom.\n" );
		delete( newAtom );
		return( NULL );
	}

	return newAtom;
}
MP_Atom_c* MP_Harmonic_Atom_Plugin_c::create_frombinary( FILE *fid, MP_Dict_c *dict)
{
  const char* func = "MP_Harmonic_Atom_c::create_frombinary(fid,mode)";
  MP_Harmonic_Atom_Plugin_c* newAtom = NULL;

  /* Instantiate and check */
  newAtom = new MP_Harmonic_Atom_Plugin_c(dict);
  if ( newAtom == NULL )
    {
      mp_error_msg( func, "Failed to create a new atom.\n" );
      return( NULL );
    }

  /* Read and check */
  if ( newAtom->init_frombinary( fid ) )
    {
      mp_error_msg( func, "Failed to read the new Harmonic atom.\n" );
      delete( newAtom );
      return( NULL );
    }

  return( (MP_Atom_c*)newAtom );
}



/********************/
/* Void constructor */
MP_Harmonic_Atom_Plugin_c::MP_Harmonic_Atom_Plugin_c( MP_Dict_c* dict ):MP_Gabor_Atom_Plugin_c(dict)
{
  numPartials         = 0;
  harmonicity         = NULL;
  partialAmpStorage   = NULL;
  partialPhaseStorage = NULL;
  partialAmp          = NULL;
  partialPhase        = NULL;
}


/************************/
/* Local allocations    */
int MP_Harmonic_Atom_Plugin_c::alloc_harmonic_atom_param( MP_Chan_t setNumChans,
    const unsigned int setNumPartials )
{

  const char* func = "MP_Harmonic_Atom_c::local_alloc(numChans,numPartials)";
  unsigned int i;
  unsigned int j;

  /* harmonicity */
  if ( (harmonicity = (MP_Real_t*)calloc(setNumPartials, sizeof(MP_Real_t)) ) == NULL )
    {
      mp_error_msg( func, "Can't allocate harmonicity.\n" );
      return( 1 );
    }
  else
    {
      for (j = 0; j < setNumPartials; j++)
        {
          *(harmonicity+j) = (MP_Real_t)(j+1);
        }
    }

  /* partial's amp */
  if ( (partialAmpStorage = (MP_Real_t*)calloc(setNumChans*setNumPartials, sizeof(MP_Real_t)) ) == NULL )
    {
      mp_warning_msg( func, "Can't allocate the partialAmpStorage array.\n" );
      return( 1 );
    }
  else if  ( (partialAmp = (MP_Real_t**) malloc(setNumChans*sizeof(MP_Real_t*)) ) == NULL )
    {
      mp_warning_msg( func, "Can't allocate the partialAmp array.\n" );
      return( 1 );
    }
  else
    {
      for (i=0; i < setNumChans; i++)
        {
          partialAmp[i] = partialAmpStorage+i*setNumPartials;
        }
    }

  /* partial's phase */
  if ( (partialPhaseStorage = (MP_Real_t*)calloc(setNumChans*setNumPartials,sizeof(MP_Real_t)) ) == NULL )
    {
      mp_warning_msg( func, "Can't allocate the partialPhaseStorage array.\n" );
      return( 1 );
    }
  else if ( (partialPhase = (MP_Real_t**)malloc(setNumChans*sizeof(MP_Real_t*)) ) == NULL )
    {
      mp_warning_msg( func, "Can't allocate the partialPhase array.\n" );
      return( 1 );
    }
  else
    {
      for (i=0; i < setNumChans; i++)
        {
          partialPhase[i] = partialPhaseStorage+i*setNumPartials;
        }
    }

  return( 0 );
}


/********************/
/* File reader      */
int MP_Harmonic_Atom_Plugin_c::init_fromxml(TiXmlElement* xmlobj)
{
  const char* func = "MP_Harmonic_Atom_c::read(fid,mode)";

  /* Go up one level */
  if ( MP_Gabor_Atom_Plugin_c::init_fromxml( xmlobj ) )
    {
      mp_error_msg( func, "Reading of Harmonic atom fails at the Gabor atom level.\n" );
      return( 1 );
    }

  /* Read at local level */
  // First, MONOPHONIC FEATURES
  // Iterate children and discover:
  TiXmlNode* kid = 0;
  TiXmlElement* kidel;
  const char* datatext;
  while((kid = xmlobj->IterateChildren(kid))){
    kidel = kid->ToElement();
    if(kidel != NULL){
      // par[type=numPartials]
      if((strcmp(kidel->Value(), "par")==0) && (strcmp(kidel->Attribute("type"), "numPartials")==0)){
        datatext = kidel->GetText();
        if(datatext != NULL){
	  numPartials = strtol(datatext, NULL, 0);
        }
      }
    }
  }
  if (numPartials <=1)
  {
          mp_error_msg( func, "Failed to read the number of partials (in text mode).\n");
          return( 1 );
  }

  /* Alloc at local level */
  if (alloc_harmonic_atom_param( numChans, numPartials ) )
  {
      mp_error_msg( func, "Allocation of Harmonic atom failed at the local level.\n" );
      return( 1 );
  }

  /* Try to read the harmonicity, partialAmp and partialPhase */
  // Then, MULTICHANNEL FEATURES
  // Iterate children and:
  kid = 0;
  unsigned int count_harm=0, count_pamp=0, count_pphase=0;
  while((kid = xmlobj->IterateChildren(kid))){
    kidel = kid->ToElement();
    if(kidel != NULL){

      //      if item is par[type=harmonicity][partial=x] then store that
      if((strcmp(kidel->Value(), "par")==0) && (strcmp(kidel->Attribute("type"), "harmonicity")==0)){
        ++count_harm;
        // Get the partial, and check bounds (could cause bad mem writes otherwise)
	datatext = kidel->Attribute("partial");
        long partial = strtol(datatext, NULL, 0);
        if((partial<0) || (partial >= numPartials)){
            mp_error_msg( func, "Found a <par type='harmonicity'> tag with partial number %i, which is outside the range for this atom [0,%i).\n", partial, numPartials);
            return( 1 );
        }
	datatext = kidel->GetText();
	harmonicity[partial] = strtod(datatext, NULL);
      }

      //      if item is harmo_gaborPar[chan=x] then scan that -- it contains the traditional gabor parameter pairs (amp, phase) PER PARTIAL as children
      if(strcmp(kidel->Value(), "harmo_gaborPar")==0){
        // Get the channel, and check bounds (could cause bad mem writes otherwise)
	datatext = kidel->Attribute("chan");
        long chan = strtol(datatext, NULL, 0);
        if((chan<0) || (chan >= numChans)){
            mp_error_msg( func, "Found a <harmo_gaborPar> tag with channel number %i, which is outside the channel range for this atom [0,%i).\n", chan, numChans);
            return( 1 );
        }
        // Now we must scan the harmo kids and process them
        TiXmlNode* harmokid = 0;
        TiXmlElement* harmokidel;
        while((harmokid = kidel->IterateChildren(harmokid))){
          harmokidel = harmokid->ToElement();
          if(harmokidel != NULL){
            //      if item is par[type=amp][partial=x] then store that
            if((strcmp(harmokidel->Value(), "par")==0) && (strcmp(harmokidel->Attribute("type"), "amp")==0)){
              ++count_pamp;
              // Get the partial, and check bounds (could cause bad mem writes otherwise)
	      datatext = harmokidel->Attribute("partial");
              long partial = strtol(datatext, NULL, 0);
              if((partial<0) || (partial >= numPartials)){
                  mp_error_msg( func, "Found a <par type='amp'> tag with partial number %i, which is outside the range for this atom [0,%i).\n", partial, numPartials);
                  return( 1 );
              }
              datatext = harmokidel->GetText();
              partialAmp[chan][partial] = strtod(datatext, NULL);
            }

            //      if item is par[type=phase][partial=x] then store that
            if((strcmp(harmokidel->Value(), "par")==0) && (strcmp(harmokidel->Attribute("type"), "phase")==0)){
              ++count_pphase;
              // Get the partial, and check bounds (could cause bad mem writes otherwise)
	      datatext = harmokidel->Attribute("partial");
              long partial = strtol(datatext, NULL, 0);
              if((partial<0) || (partial >= numPartials)){
                  mp_error_msg( func, "Found a <par type='phase'> tag with partial number %i, which is outside the range for this atom [0,%i).\n", partial, numPartials);
                  return( 1 );
              }
              datatext = harmokidel->GetText();
              partialPhase[chan][partial] = strtod(datatext, NULL);
            }
          }
        } // end iteration over harmo_gaborPar kids
      } // end harmo_gaborPar
    }
  }

  // Finally check counts -- harmonicity one per partial, the others one per partial-times-channel
  if(count_harm != numPartials){
    mp_error_msg( func, "Scanned an atom with %i partials, but failed to get that number of 'harmonicity' values (%i).\n",
		numPartials, count_harm);
    return( 1 );
  }
  if(count_pamp != numPartials * numChans){
    mp_error_msg( func, "Scanned an atom with %i partials and %i channels, but failed to get that number of partial 'amp' values (%i).\n",
		numPartials, numChans, count_pamp);
    return( 1 );
  }
  if(count_pphase != numPartials * numChans){
    mp_error_msg( func, "Scanned an atom with %i partials and %i channels, but failed to get that number of partial 'phase' values (%i).\n",
		numPartials, numChans, count_pphase);
    return( 1 );
  }

  return 0;
}

int MP_Harmonic_Atom_Plugin_c::init_frombinary( FILE *fid )
{
  const char* func = "MP_Harmonic_Atom_c::read(fid,mode)";
  //char line[MP_MAX_STR_LEN];
  //char str[MP_MAX_STR_LEN];
  //  double fidHarmonicity, fidAmp,fidPhase;
    unsigned int i; //, iRead;
    unsigned int j; //, jRead;

  /* Go up one level */
  if ( MP_Gabor_Atom_Plugin_c::init_frombinary( fid ) )
    {
      mp_error_msg( func, "Reading of Harmonic atom fails at the Gabor atom level.\n" );
      return( 1 );
    }

  /* Read at local level */
      /* Read the number of partials */
      if ( ( mp_fread( &numPartials,  sizeof(unsigned int), 1, fid ) != 1) ||
           (numPartials <=1) )
        {
          mp_error_msg( func, "Failed to read the number of partials (in binary mode).\n");
          return( 1 );
        }

  /* Alloc at local level */
  if (alloc_harmonic_atom_param( numChans, numPartials ) )
    {
      mp_error_msg( func, "Allocation of Harmonic atom failed at the local level.\n" );
      return( 1 );
    }

  /* Try to read the harmonicity, partialAmp and partialPhase */
      /* Try to read the harmonicity, partialAmp, partialPhase */
      if ( mp_fread( harmonicity,   sizeof(MP_Real_t), numPartials, fid ) != (size_t)numPartials )
        {
          mp_warning_msg( func, "Failed to read the harmonicity array.\n" );
          for ( j=0; j<numPartials; j++ ) *(harmonicity+j) = (MP_Real_t)(j+1);
        }

      if ( mp_fread( partialAmpStorage,   sizeof(MP_Real_t), numChans*numPartials, fid ) != (size_t)(numChans*numPartials) )
        {
          mp_warning_msg( func, "Failed to read the partialAmp array.\n" );
          for ( i=0; i<numChans; i++)
            {
              for ( j=0; j<numPartials; j++)
                {
                  *(partialAmp[i]+j) = 0.0;
                }
            }
        }
      if ( mp_fread( partialPhaseStorage, sizeof(MP_Real_t), numChans*numPartials, fid ) != (size_t)(numChans*numPartials) )
        {
          mp_warning_msg( func, "Failed to read the partialPhase array.\n" );
          for ( i=0; i<numChans; i++)
            {
              for ( j=0; j<numPartials; j++)
                {
                  *(partialPhase[i]+j) = 0.0;
                }
            }
        }

  return( 0 );
}

/**************/
/* Destructor */
MP_Harmonic_Atom_Plugin_c::~MP_Harmonic_Atom_Plugin_c()
{
#ifndef NDEBUG
  fprintf( stderr, "mplib DEBUG -- ~MP_Harmonic_Atom_c() - Deleting harmonic_atom.\n" );
#endif
  free(harmonicity);
  free(partialAmpStorage);
  free(partialPhaseStorage);
  free(partialAmp);
  free(partialPhase);
#ifndef NDEBUG
  fprintf( stderr, "done.\n" );
#endif
}


/***************************/
/* OUTPUT METHOD           */
/***************************/

int MP_Harmonic_Atom_Plugin_c::write( FILE *fid, const char mode )
{

  unsigned int i = 0;
  int nItem = 0;
  unsigned int j;

  /* Call the parent's write function */
  nItem += MP_Gabor_Atom_Plugin_c::write( fid, mode );

  /* Print the other harmonic-specific parameters */
  switch ( mode )
    {

    case MP_TEXT:
      /* Number of partials */
      nItem += fprintf( fid, "\t\t<par type=\"numPartials\">%u</par>\n", numPartials );
      /* Harmonicity */
      for (j = 0; j < numPartials; j++)
        {
          nItem += fprintf( fid, "\t\t<par type=\"harmonicity\" partial=\"%u\">%lg</par>\n",j,(double)harmonicity[j] );
        }
      /* partialAmp and partialPhase */
      for (i = 0; i<numChans; i++)
        {
          nItem += fprintf( fid, "\t\t<harmo_gaborPar chan=\"%u\">\n", i );
          for (j = 0; j < numPartials; j++)
            {
              nItem += fprintf( fid, "\t\t<par type=\"amp\" partial=\"%u\">%lg</par>\n",j,(double)partialAmp[i][j] );
              nItem += fprintf( fid, "\t\t<par type=\"phase\" partial=\"%u\">%lg</par>\n",j,(double)partialPhase[i][j] );
            }
          nItem += fprintf( fid, "\t\t</harmo_gaborPar>\n" );
        }
      break;

    case MP_BINARY:
      /* Number of partials */
      nItem += (int)mp_fwrite( &numPartials,  sizeof(unsigned int), 1, fid );
      /* Binary parameters */
      nItem += (int)mp_fwrite( harmonicity,   sizeof(MP_Real_t), numPartials, fid );
      nItem += (int)mp_fwrite( partialAmpStorage,   sizeof(MP_Real_t), numChans*numPartials, fid );
      nItem += (int)mp_fwrite( partialPhaseStorage, sizeof(MP_Real_t), numChans*numPartials, fid );

      break;

    default:
      break;
    }

  return( nItem );
}


/***************************/
/* OTHER METHODS           */
/***************************/

/*************/
/* Type name */
const char * MP_Harmonic_Atom_Plugin_c::type_name(void)
{
  return ("harmonic");
}

/**********************/
/* Readable text dump */
int MP_Harmonic_Atom_Plugin_c::info( FILE *fid )
{

  int nChar = 0;
  FILE* bakStream;
  void (*bakHandler)( void );

  /* Backup the current stream/handler */
  bakStream = get_info_stream();
  bakHandler = get_info_handler();
  /* Redirect to the given file */
  set_info_stream( fid );
  set_info_handler( MP_FLUSH );
  /* Launch the info output */
  nChar += info();
  /* Reset to the previous stream/handler */
  set_info_stream( bakStream );
  set_info_handler( bakHandler );

  return( nChar );
}

/**********************/
/* Readable text dump */
int MP_Harmonic_Atom_Plugin_c::info()
{
	unsigned int i = 0;
	int nChar = 0;
	unsigned int j;

	nChar += (int)mp_info_msg( "HARMONIC ATOM", "%s window (window opt=%g)\n", window_name(windowType), windowOption );
	nChar += (int)mp_info_msg( "        |-", "[%d] channel(s), [%u] partials\n", numChans, numPartials );
	nChar += (int)mp_info_msg( "        |-", "Freq %g\tChirp %g\n", (double)freq, (double)chirp);
	for ( i=0; i<numChans; i++ )
    {
		nChar += (int)mp_info_msg( "        |-", "(%d/%d)\tSupport= %lu %lu\tAmp %g\tPhase %g\n", i+1, numChans, support[i].pos, support[i].len, (double)amp[i], (double)phase[i] );
		for ( j=0; j<numPartials; j++)
        {
			nChar += (int)mp_info_msg( "        |-", "\t[%g]\tAmp %g\tPhase %g\n", (double)harmonicity[j], (double)partialAmp[i][j], (double)partialPhase[i][j] );
        }
    }
	
	return( nChar );
}

/********************/
/* Waveform builder */
void MP_Harmonic_Atom_Plugin_c::build_waveform( MP_Real_t *outBuffer )
{

  MP_Real_t *window;
  MP_Real_t *atomBuffer;
  /*unsigned long int windowCenter = 0;*/
  /* Parameters for the atom waveform : */
  MP_Chan_t chanIdx;
  unsigned int t;
  unsigned long int len;
  unsigned int j;
  double dHalfChirp, dAmp, dFreq, dPhase, dT, dGlobPhase, dGlobAmp;
  double argument;
  
  assert( outBuffer != NULL );

  /* Dereference the arguments once and for all */
  dHalfChirp = (double)( chirp ) * MP_PI; /* chirp/2 */
  dFreq      = (double)(  freq ) * MP_2PI;

  for ( chanIdx = 0 , atomBuffer = outBuffer;
        chanIdx < numChans;
        chanIdx++  )
    {
      /* Dereference the atom length in the current channel once and for all */
      len = support[chanIdx].len;

      /* Make the window */
      /*windowCenter = */ MPTK_Server_c::get_win_server()->get_window( &window, len, windowType, windowOption );
      assert( window != NULL );

      /* Dereference the arguments once and for all */
      dGlobAmp   = (double)(   amp[chanIdx] );
      dGlobPhase = (double)( phase[chanIdx] );

      /* 1/ Build the desired modulation (without multiplying by the window)
       * \f[
       * \sum_{k=1}^{\mbox{numPartials}} a_k
       * \cdot \cos\left(2\pi \lambda_k \left(\mbox{chirp} \cdot \frac{t^2}{2}
       *      + \mbox{freq} \cdot t\right)+ \mbox{phase} + \phi_k\right)
       * \f]
       */

      for ( t = 0; t<len; t++ )
        {

          /* Compute the cosine's argument */
          dT = (double)(t);
          argument = (dHalfChirp*dT + dFreq)*dT;
          /* The above does:
           * argument = dHalfChirp*dT*dT + dFreq*dT but saves a multiplication.
           * \todo save multiplications by integrating twice the second derivative ?
          */
          /* -- first partial */
          dAmp   = dGlobAmp*(double)(   partialAmp[chanIdx][0] );
          dPhase = (double)( partialPhase[chanIdx][0] );
          *(atomBuffer+t) = ( dAmp * cos( (harmonicity[0]*argument) +
                                          dGlobPhase+dPhase) );

          /* -- following partials */
          for ( j = 1; j < numPartials; j++)
            {
              dAmp   = dGlobAmp*(double)(   partialAmp[chanIdx][j] );
              dPhase = (double)( partialPhase[chanIdx][j] );
              *(atomBuffer+t) += ( dAmp * cos( (harmonicity[j]*argument) +
                                               dGlobPhase+dPhase) );
            }
        } /* <-- end loop on samples */

      /* 2/ multiply by the window and the global amplitude */
      for ( t = 0; t<len; t++ )
        {
          *(atomBuffer+t)   *= *(window+t);
        }

      /* Go to the next channel */
      atomBuffer += len;
    }

}


/* Adds a pseudo Wigner-Ville of the atom to a time-frequency map */
int MP_Harmonic_Atom_Plugin_c::add_to_tfmap( MP_TF_Map_c *tfmap, const char tfmapType )
{
  const char* func = "MP_Harmonic_Atom_Plugin_c::add_to_tfmap( MP_TF_Map_c *tfmap, const char tfmapType )";
  static MP_Gabor_Atom_Plugin_c *gatom = NULL; // A Gabor atom used to plot each partial of the harmonic atom. Implemented as static to be allocated and initialized only once
  static MP_Chan_t nchans = 0; // maximum number of channels the gabor atom was ever allocated. May differ from gatom->numChans
  unsigned int k;
  MP_Chan_t chanIdx;
  char flag = 0;


  if (gatom==NULL)
    { // first allocation of the gabor atom
      MP_Atom_c* (*emptyAtomCreator)( MP_Dict_c* dict ) = MP_Atom_Factory_c::get_empty_atom_creator("gabor");
      if (NULL == emptyAtomCreator)
        {
          mp_error_msg( func, "Gabor atom is not registred in the atom factory" );
          return( 0 );
        }

      if ( (gatom =  (MP_Gabor_Atom_Plugin_c*)(*emptyAtomCreator)(dict))  == NULL )
        {
          mp_error_msg( func, "Can't create a new Gabor atom.\n" );
          return( 0 );
        }
   if ( gatom->alloc_atom_param( numChans) )
        {
          mp_error_msg( func, "Failed to allocate some vectors in the new atom. Returning a NULL atom.\n" );
          return( 0 );

        }
      if ( gatom->alloc_gabor_atom_param( numChans) )
        {
          mp_error_msg( func, "Failed to allocate some vectors in the new Gabor atom. Returning a NULL atom.\n" );
          return( 0 );

        }
      /* Set the window-related values */
      if ( window_type_is_ok( windowType ) )
        {
          gatom->windowType   = windowType;
          gatom->windowOption = windowOption;
        }
      else
        {
          mp_error_msg( func, "The window type is unknown. Returning a NULL atom.\n" );

          return( 0 );
        }
      nchans = numChans;
    }
  else if ( nchans<numChans)
    { // reallocation if more channels are needed
      delete gatom;
      MP_Atom_c* (*emptyAtomCreator)( MP_Dict_c* dict ) = MP_Atom_Factory_c::get_empty_atom_creator("gabor");
      if (NULL == emptyAtomCreator)
        {
          mp_error_msg( func, "Gabor atom is not registred in the atom factory" );
          return( 0 );
        }

      if ( (gatom =  (MP_Gabor_Atom_Plugin_c*)(*emptyAtomCreator)(dict))  == NULL )
        {
          mp_error_msg( func, "Can't create a new Gabor atom.\n" );
          return( 0 );
        }

      if ( gatom->alloc_atom_param( numChans) )
        {
          mp_error_msg( func, "Failed to allocate some vectors in the new atom. Returning a NULL atom.\n" );
          return( 0 );

        }
      if ( gatom->alloc_gabor_atom_param( numChans) )
        {
          mp_error_msg( func, "Failed to allocate some vectors in the new Gabor atom. Returning a NULL atom.\n" );
          return( 0 );

        }
      /* Set the window-related values */
      if ( window_type_is_ok( windowType ) )
        {
          gatom->windowType   = windowType;
          gatom->windowOption = windowOption;
        }

      else
        {
          mp_error_msg( func, "The window type is unknown. Returning a NULL atom.\n" );

          return( 0 );
        }
      nchans = numChans;
    }
  else
    { /* possible decrease of gatom->numChans, keeping track of
      	      the actually allocated number of channels nchans. */
      gatom->numChans = numChans;
      gatom->windowType = windowType;
      gatom->windowOption = windowOption;
    }
  // Setting the partials support
  for (chanIdx=0; chanIdx< numChans; chanIdx++)
    {
      gatom->support[chanIdx].pos = support[chanIdx].pos;
      gatom->support[chanIdx].len = support[chanIdx].len;
    }

  // Plotting each partial
  for (k = 0; k < numPartials; k++)
    {
      // Setting the partial freq, chirp
      gatom->freq = harmonicity[k]*freq;
      gatom->chirp = harmonicity[k]*chirp;
      // Setting the amplitude and phase on each channel
      for (chanIdx = 0; chanIdx < numChans; chanIdx++)
        {
          gatom->amp[chanIdx] = amp[chanIdx]*partialAmp[chanIdx][k];
          gatom->phase[chanIdx] = phase[chanIdx]+partialPhase[chanIdx][k];
        }
      // Plotting, and counting whether something was plotted or not
      flag = flag || gatom->add_to_tfmap(tfmap, tfmapType);
    }
  return( flag );
}

/***********************************************************************/
/* Sorting function which characterizes various properties of the atom,
   along one channel */
int MP_Harmonic_Atom_Plugin_c::has_field( int field )
{

  if (MP_Gabor_Atom_Plugin_c::has_field( field ) ) return ( MP_TRUE );
  else switch (field)
      {
      case MP_NUMPARTIALS_PROP :
        return( MP_TRUE );
      case MP_HARMONICITY_PROP :
        return( MP_TRUE );
      case MP_PARTIAL_AMP_PROP :
        return( MP_TRUE );
      case MP_PARTIAL_PHASE_PROP :
        return( MP_TRUE );
      default :
        return( MP_FALSE );
      }
}

MP_Real_t MP_Harmonic_Atom_Plugin_c::get_field( int field, MP_Chan_t chanIdx )
{
  MP_Real_t x;
  unsigned int c,h; /* chan, partials */

  if (MP_Gabor_Atom_Plugin_c::has_field( field ) ) return ( MP_Gabor_Atom_Plugin_c::get_field(field,chanIdx) ); 
  else switch (field)
    {
    case MP_NUMPARTIALS_PROP :
      x = (MP_Real_t) numPartials;
      printf("numPartials = %d",(unsigned int) x);

      break;
    case MP_HARMONICITY_PROP :
      x = (MP_Real_t)(harmonicity[chanIdx]); //! chanIdx refers to index of partial
      break;
    case MP_PARTIAL_AMP_PROP :
      //! chanIdx conversion -> [chanIdx][partialIdx] (to be checked ...)
      h = chanIdx / numChans;
      c = chanIdx % numChans;
      x = (MP_Real_t)(partialAmp[c][h]); 
      break;
    case MP_PARTIAL_PHASE_PROP :
      //! chanIdx conversion -> [chanIdx][partialIdx] (to be checked ...)
      h = chanIdx / numChans;
      c = chanIdx % numChans;
      x = (MP_Real_t)(partialPhase[c][h]);
      break;
    default :
      mp_warning_msg( "MP_Harmonic_Atom_Plugin_c::get_field()", "Unknown field. Returning ZERO.\n" );
      x = 0.0;
    }
  
  return( x );
  
}



//
// Registry symbol create to export the adress of creator of all the classes
//

/*
DLL_EXPORT void registry(void)
{
  
}
*/
