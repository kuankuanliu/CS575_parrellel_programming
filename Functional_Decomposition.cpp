#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <iostream>
#include <fstream>
#include <cmath>


int	NowYear;		// 2023 - 2028
int	NowMonth;		// 0 - 11

float	NowPrecip;		// inches of rain per month
float	NowTemp;		// temperature this month
float	NowHeight;		// rye grass height in inches
int	    NowNumRabbits;		// number of rabbits in the current population
int     NowNumZombie;           //current Zomebie population


const float RYEGRASS_GROWS_PER_MONTH =		20.0;
const float ONE_RABBITS_EATS_PER_MONTH =	 1.0;

const float AVG_PRECIP_PER_MONTH =	       12.0;	// average
const float AMP_PRECIP_PER_MONTH =		4.0;	// plus or minus
const float RANDOM_PRECIP =			2.0;	// plus or minus noise

const float AVG_TEMP =				60.0;	// average
const float AMP_TEMP =				20.0;	// plus or minus
const float RANDOM_TEMP =			10.0;	// plus or minus noise

const float MIDTEMP =				60.0;
const float MIDPRECIP =				14.0;

unsigned int seed = 10;
float
Ranf( unsigned int *seedp,  float low, float high )
{
        float r = (float) rand_r( seedp );              // 0 - RAND_MAX

        return(   low  +  r * ( high - low ) / (float)RAND_MAX   );
}

float
Sqr( float x )
{
        return x*x;
}

void Rabbits(){
    while(NowYear < 2029){
        int nextNumRabbits = NowNumRabbits;
        int carryingCapacity = (int)( NowHeight );
        if( nextNumRabbits < carryingCapacity )
                nextNumRabbits++;
        else
                if( nextNumRabbits > carryingCapacity )
                        nextNumRabbits--;

        if( nextNumRabbits < 0 )
        nextNumRabbits = 0;

        //Done Computing Barrier
        #pragma omp barrier
        NowNumRabbits = nextNumRabbits;

        //Done Assigning Barrier
        #pragma omp barrier

        //Done Printing Barrier
        #pragma omp barrier
    }
}

void  RyeGrass(){
        while(NowYear < 2029){
                float nextheight = NowHeight;
                float tempFactor = exp(   -Sqr(  ( NowTemp - MIDTEMP ) / 10.  )   );
                float precipFactor = exp(   -Sqr(  ( NowPrecip - MIDPRECIP ) / 10.  )   );
                nextheight += tempFactor * precipFactor * RYEGRASS_GROWS_PER_MONTH;
                nextheight -= (float)NowNumRabbits * ONE_RABBITS_EATS_PER_MONTH;
                if(nextheight < 0.){
                nextheight = 0.;
                }
        
                //Done Computing Barrier
                #pragma omp barrier
                NowHeight = nextheight;
        
                //Done Assigning Barrier
                #pragma omp barrier
        
                //Done Printing Barrier
                #pragma omp barrier
        }
}

void Watcher( ){
        while(NowYear < 2029){
                //Done Computing Barrier
                #pragma omp barrier
        
                //Done Assigning Barrier
                #pragma omp barrier
                float C_temperature = (5./9.) * (NowTemp - 32);
                float I_Height = NowPrecip  * 2.54;
                        printf("%d/%02d, %6.2f,%6.2f,%02d,%6.2f,%02d\n", NowYear, NowMonth+1,C_temperature, I_Height, NowNumRabbits, NowHeight, NowNumZombie);
                        float ang = (  30.*(float)NowMonth + 15.  ) * ( M_PI / 180. );

                        float temp = AVG_TEMP - AMP_TEMP * cos( ang );
                        NowTemp = temp + Ranf( &seed, -RANDOM_TEMP, RANDOM_TEMP);

                        float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin( ang );
                        NowPrecip = precip + Ranf( &seed, -RANDOM_PRECIP, RANDOM_PRECIP);


                        if(NowPrecip < 0.)
                             NowPrecip = 0.;

                        NowMonth++;
                        if(NowMonth > 11){
                                NowYear++;
                                NowMonth = 0;
                        }
                //Done Printing Barrier
                #pragma omp barrier
        }
}

//Zombie eat rabbits and might dead by cycle
void Zomebie(){
    while(NowYear < 2029){
        float tmpZombie = NowNumZombie;

        if(tmpZombie  >= (NowNumRabbits))
            tmpZombie = tmpZombie -2;
        else if(tmpZombie  < (NowNumRabbits/3)) 
                 tmpZombie = tmpZombie + 4;
        
        if(NowNumZombie<0)
        {
            NowNumZombie=0;
        }
        
       

        //Done Computing Barrier
        #pragma omp barrier

        NowNumZombie = tmpZombie;

        //Done Assigning Barrier
        #pragma omp barrier


        //Done Printing Barrier
        #pragma omp barrier
    }
}

int main( int argc, char* argv[])
{
    #ifndef _OPENMP
    fprintf( stderr, "OpenMP is not supported here.\n");
    return 1;
    #endif


    // starting date and time:
    NowMonth =    0;
    NowYear  = 2023;
    // starting state:
    NowNumRabbits = 1;
    NowNumZombie = 4;
    NowHeight =  5.;

    float ang = (  30.*(float)NowMonth + 15.  ) * ( M_PI / 180. );

    float temp = AVG_TEMP - AMP_TEMP * cos( ang );
    NowTemp = temp + Ranf( &seed, -RANDOM_TEMP, RANDOM_TEMP );

    float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin( ang );
    NowPrecip = precip + Ranf( &seed, -RANDOM_PRECIP, RANDOM_PRECIP );
    if ( NowPrecip < 0. )
        NowPrecip = 0.;


    omp_set_num_threads( 4 );
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            Rabbits( );
        }

        #pragma omp section
        {
            RyeGrass( );
        }

        #pragma omp section
        {
            Watcher( );
        }

        #pragma omp section
        {
            Zomebie( );
        }
    }     

    return 0;

}