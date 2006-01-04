/******************************************************************************/
/*                                                                            */
/*                      make_regression_constants.cpp                         */
/*                                                                            */
/*                        Matching Pursuit ToolKit                            */
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
/*
 * SVN log:
 *
 * $Author: sacha $
 * $Date: 2005-08-13 16:49:56 +0200 (Sat, 13 Aug 2005) $
 * $Revision: 27 $
 *
 */

/*
   This program is not a utility per se. It is used to generate
   the regression_constants.h header at build time.
*/

#include "mp_system.h"

#define MP_MAX_REGRESSION_SIZE 128

int main( void ) {

  int l;
  double LLp1, x2L, x2Lp1, x2Lm1x2Lp3, LLL;
  double C0[MP_MAX_REGRESSION_SIZE];
  double minus_C1[MP_MAX_REGRESSION_SIZE];
  double C2[MP_MAX_REGRESSION_SIZE];
  FILE *fid;

  C0[0] = minus_C1[0] = C2[0] = 0.0; /* <- Phoney value to avoid div by 0 */
  for ( l = 1; l < MP_MAX_REGRESSION_SIZE; l++ ) {

    LLp1 = l*( l + 1 );
    x2L = 2*l;
    x2Lp1 = x2L + 1;
    x2Lm1x2Lp3 = ( x2L - 1 )*( x2L + 3 );
    LLL = x2Lm1x2Lp3 * x2Lp1;
    
    C0[l] = 45.0 / (LLp1 * LLL);
    minus_C1[l] = - 15.0 / LLL;
    C2[l] = 3 / (LLp1 * x2Lp1);
  }

  /* Output the tables */
  if ( !( fid = fopen( "regression_constants.h", "w" ) ) ) {
    fprintf( stderr, "Can't open file regression_constants.h for writing.\n" );
    return( 1 );
  }

  fprintf( fid, "/* This file is automatically generated by program make_regression_constants\n"
	   " at build time. Do not edit it by hand. */\n" );
  fprintf( fid, "\n" );
  fprintf( fid, "#ifndef __regressions_constants_h_\n" );
  fprintf( fid, "#define __regressions_constants_h_\n" );
  fprintf( fid, "\n" );
  fprintf( fid, "#define MP_MAX_REGRESSION_SIZE %d\n", MP_MAX_REGRESSION_SIZE );
  fprintf( fid, "\n" );
  fprintf( fid, "#define MP_C0_1 %.32f\n", C0[1] );
  fprintf( fid, "#define MP_minus_C1_1 %.32f\n", minus_C1[1] );
  fprintf( fid, "#define MP_C2_1 %.32f\n", C2[1] );
  fprintf( fid, "\n" );
  fprintf( fid, "#define MP_C0_2 %.32f\n", C0[2] );
  fprintf( fid, "#define MP_minus_C1_2 %.32f\n", minus_C1[2] );
  fprintf( fid, "#define MP_C2_2 %.32f\n", C2[2] );
  fprintf( fid, "\n" );
  fprintf( fid, "#define MP_C0_3 %.32f\n", C0[3] );
  fprintf( fid, "#define MP_minus_C1_3 %.32f\n", minus_C1[3] );
  fprintf( fid, "#define MP_C2_3 %.32f\n", C2[3] );
  fprintf( fid, "\n" );
  fprintf( fid, "#define MP_C0_4 %.32f\n", C0[4] );
  fprintf( fid, "#define MP_minus_C1_4 %.32f\n", minus_C1[4] );
  fprintf( fid, "#define MP_C2_4 %.32f\n", C2[4] );
  fprintf( fid, "\n" );
  fprintf( fid, "#define MP_C0_5 %.32f\n", C0[5] );
  fprintf( fid, "#define MP_minus_C1_5 %.32f\n", minus_C1[5] );
  fprintf( fid, "#define MP_C2_5 %.32f\n", C2[5] );
  fprintf( fid, "\n" );
  fprintf( fid, "\n" );
  fprintf( fid, "\n" );
  fprintf( fid, "#define MP_C0_Table { \\\n" );
  fprintf( fid, "0.0, \\\n" ); /* <- This is a phoney value to avoid the division by 0. */
  for ( l = 1; l < MP_MAX_REGRESSION_SIZE; l++ ) {
    fprintf( fid, "%.32f, \\\n", C0[l] );
  }
  fprintf( fid, "}\n" );
  fprintf( fid, "\n" );
  fprintf( fid, "#define MP_minus_C1_Table { \\\n" );
  fprintf( fid, "0.0, \\\n" ); /* <- This is a phoney value to avoid the division by 0. */
  for ( l = 1; l < MP_MAX_REGRESSION_SIZE; l++ ) {
    fprintf( fid, "%.32f, \\\n", minus_C1[l] );
  }
  fprintf( fid, "}\n" );
  fprintf( fid, "\n" );
  fprintf( fid, "#define MP_C2_Table { \\\n" );
  fprintf( fid, "0.0, \\\n" ); /* <- This is a phoney value to avoid the division by 0. */
  for ( l = 1; l < MP_MAX_REGRESSION_SIZE; l++ ) {
    fprintf( fid, "%.32f, \\\n", C2[l] );
  }
  fprintf( fid, "}\n" );
  fprintf( fid, "\n" );
  fprintf( fid, "\n" );
  fprintf( fid, "#endif /* __regressions_constants_h_ */\n" );

  fclose( fid );

  return( 0 );
}