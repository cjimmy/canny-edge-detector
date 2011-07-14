 #include<iostream>
#include<cstdlib>
#include "EasyBMP.h"
#include<math.h>

using namespace std;


#define MAX_SIZE 5
#define PI 3.14159
#define T_LOW 65
#define T_HIGH 80

int importImg(char*);
void outputImg(void);
void printFileInfo(BMP);
float convolve(int[][5], int,  float, int, int);
void gaussianBlue(void);
void sobel(void);
void noMax(void);
void hysteresis(void);
bool isFirstMax(int , int , int );
int getOrientation(float);
bool isBetween(float, float, float, float, float);



int **imageArray;
float  **thetas;
int** magArray;

unsigned int ROWS;
unsigned int COLUMNS;
char DEPTH;


//*****************************
// prints size of img and depth of bits
//*****************************/

void printFileInfo(BMP image){
  cout << endl << "File info:" << endl;
  cout << image.TellWidth() << " x " << image.TellHeight()
       << " at " << image.TellBitDepth() << " bpp" << endl << endl;
}


//***************************** 
// uses the EasyBMP libary to import a bmp image
// and then takes the RGB and calulates a brightness
// assigning it to another array that we can use for 
// further manipulation
// The function essentially loads the file of the name given
// and then desaturates it
//*****************************

int importImg(char* filename){
  BMP InputIMG;
  cout << "Starting BMP final code" << endl;
  
  cout << "Open File: " << filename << endl;
  if(!InputIMG.ReadFromFile(filename)){
    cout << "Invalid File Name..." << endl;
    return EXIT_FAILURE;
  }

  printFileInfo(InputIMG);
  COLUMNS = InputIMG.TellWidth();  // num cols
  ROWS = InputIMG.TellHeight(); // num rows
  DEPTH = InputIMG.TellBitDepth();
  
  //allocate Memory
  imageArray = new int *[COLUMNS] ; // row memory allocation
  for( int i = 0 ; i < COLUMNS ; i++ ){ // column memory allocation
    imageArray[i] = new int[ROWS];
  }

  thetas = new float *[COLUMNS];
  for(int i = 0; i < COLUMNS; i++) {
    thetas[i] = new float[ROWS];
  }

  magArray = new int *[COLUMNS];
  for(int i=0; i < COLUMNS; i++) {
    magArray[i] = new int[ROWS];
  }

  int Temp;
  cout<< "Saving Brightness values" << endl;
  for( int j=0 ; j < ROWS ; j++)
    {
      for( int i=0 ; i < COLUMNS ; i++)
	{
	  Temp = (int) floor( 0.299*InputIMG(i,j)->Red +
			      0.587*InputIMG(i,j)->Green +
			      0.114*InputIMG(i,j)->Blue );
	  imageArray[i][j] = Temp;
	}
    }

}
//*******************
// outputs the image to a bmp
// by writing the value of the array we have 
// to each of RG, and B
//*******************
void outputImg(void){
  //setup Output IMG
  BMP OutputIMG;
  
  int byte;
  OutputIMG.SetBitDepth(DEPTH);
  OutputIMG.SetSize(COLUMNS,ROWS);
  
  cout<< "Output Image to File" << endl;
  for( int j=0 ; j < ROWS ; j++)
    {
      for( int i=0 ; i < COLUMNS ; i++)
	{
	  byte =  imageArray[i][j];
	  OutputIMG(i,j)->Red = byte;
	  OutputIMG(i,j)->Green = byte;
	  OutputIMG(i,j)->Blue = byte;
	}
    }
  
  OutputIMG.WriteToFile("Output.bmp");
  cout << "\n**** NOW GO OPEN Output.BMP ******" << endl;
}


//***************************
// convolve is a general helper funciton that applies a convolution
// to the image and then returns the weighted sum so that
// it can replace whatever pixel we were just analyzing
//**************************
float convolve(int con[][MAX_SIZE], int dim,  float divisor, int i, int j) {
    int midx = dim/2;
    int midy = dim/2;

    float weightedSum = 0;
    for(int x = i-midx; x < i + dim-midx; x++) {
      for(int y = j-midy; y < j + dim-midy; y++) {
	weightedSum += divisor*(double)(con[x-i+midx][y-j+midy]*imageArray[x][y]);
      }
    }
    return weightedSum;
}


//*****************************
// gaussian blur
// applies a gaussian blur via a convolution of a gaussian
// matrix with sigma = 1.4. hard-coded in.
// future development of can generate the gauss matrix on the fly
//*****************************
void gaussianBlur(void) {
  //define gauss matrix
  int gaussArray[5][5] = {  {2, 4, 5, 4, 2},
			  {4, 9, 12,9, 4},
			  {5, 12, 15, 12, 5},
			  {4, 9, 12,9, 4},
			  {2, 4, 5, 4, 2} };

  float gaussDivisor = 1.0/159.0;
  float sum = 0.0;
  int dim = 5;

  for(int i=2; i < COLUMNS-2; i++) {
    for(int j=2; j < ROWS-2; j++) {
      sum = convolve(gaussArray,dim, gaussDivisor, i, j);
      imageArray[i][j] = (int)sum;
    }
  }
}



//****************************
// Applies a sobel filter to find the gradient direction
// and magnitude. those values are then stored in thetas and magArray
// so that info can be used later for further analysis
//****************************
void sobel(void) {
  float sum = 0.0;
  float G_x, G_y, G, theta;
  int sobel_y[5][5] = {  {-1, 0, 1,0,0},
			 {-2, 0, 2,0,0},
			 {-1, 0, 1,0,0}, 
			 {0, 0, 0, 0, 0},
			 {0, 0, 0, 0, 0}};
			  
  int sobel_x[5][5] = {  {1, 2, 1, 0, 0},
			 {0, 0, 0, 0, 0},
			 {-1, -2, -1, 0, 0},
			 {0, 0, 0, 0, 0},
			 {0, 0, 0, 0, 0} };
			
  int dim = 3;
  //columns and rows might be mixed up here
  for (int i = 1; i < COLUMNS-1; i++ ) {
    for (int j = 1; j < ROWS-1; j++ ) {
      G_x = convolve(sobel_x, dim, 1, i, j);
      G_y = convolve(sobel_y, dim, 1, i, j);
      G = sqrt(G_x*G_x + G_y*G_y);
     // cout << G << "\n";
      // cout << atan2(G_y, G_x) << "\n";
      thetas[i][j] = getOrientation(180.0*atan2(G_y, G_x)/PI);
      //	cout << thetas[i][j] << endl;
      magArray[i][j] = G;
    }
  }
}

//***************************
// helper function that returns true if a>b and c
//***************************
bool isFirstMax(int a, int b, int c){
  if(a>b && a>c){
       return true;
  }
  return 0;
}


//****************************
// buckets the thetas into 0, 45, 90, 135
//****************************

int getOrientation(float angle) {
  if(isBetween(angle, -22.5, 22.5, -180, -157.5 ) || isBetween(angle, 157.5, 180, -22.5, 0))
    return 0;
  if(isBetween(angle, 22.5, 67.5, -157.5, -112.5))
    return 45;
  if(isBetween(angle, 67.5, 112.5, -112.5, -67.5))
    return 90;
  if(isBetween(angle, 112.5, 157.5, -67.5, -22.5))
    return 135; 
    
  return -1;

}

//*****************************
//helper function that says whether arg is between a-b or c-d
//*****************************
bool isBetween(float arg, float a, float b, float c, float d) {
  if((arg >= a && arg <= b) || (arg >= c && arg <= d)) {
    return true;
  } else {
    return false;
  }
}


//*****************************
//non-maximum suppression 
//depending on the orientation, pixels are either thrown away or accepted
//by checking it's neighbors
//*****************************
void noMax(void){
	int theta = 0;
	for( int j=1 ; j < ROWS-1 ; j++)
	 {
	    for( int i=1 ; i < COLUMNS-1 ; i++)
	     {
			theta = (int) thetas[i][j];
			switch(theta){
				case 0:
					if(isFirstMax(magArray[i][j],magArray[i+1][j],magArray[i-1][j])){
						imageArray[i][j] = 0; // black
					}
					else{
						imageArray[i][j] = 255; // white
					}
				break;
				
				case 45:
					if(isFirstMax(magArray[i][j],magArray[i+1][j+1],magArray[i-1][j-1])){
						imageArray[i][j] = 0; // black
					}
					else{
						imageArray[i][j] = 255; // white
					}
				
				break;
				
				case 90:
					if(isFirstMax(magArray[i][j],magArray[i][j+1],magArray[i][j-1])){
						imageArray[i][j] = 0; // black
					}
					else{
						imageArray[i][j] = 255; // white
					}
				break;
				
				case 135:
					if(isFirstMax(magArray[i][j],magArray[i+1][j-1],magArray[i-1][j+1])){
						imageArray[i][j] = 0; // black
					}
					else{
						imageArray[i][j] = 255; // white
					}
				break;
				
				default:
				  //	cout << "error in nomax()"<< endl;
				break;
			}
		}
	}
}




//***********************************
// main loop
// reads in the command line arguments
// then runs through the Canny Edge Detection algorithm
//***********************************

int main(int argc, char *argv[]) {
  if(argc != 2) {
    cout << "To execute: type ./BMP <file of bmp image> \n try sf.bmp, duke.bmp, lena.bmp, or tiger.bmp" << endl;
    return EXIT_FAILURE;
  }
    
    //read in array and store it as global var
    importImg(argv[1]); // also desaturates
    gaussianBlur();
    sobel();
    noMax();
    hysteresis();
    outputImg();

    // free memory                                                                        
    for( int i = 0 ; i < ROWS ; i++ ){
      delete [] imageArray[i];                                                                                                 
      delete [] thetas[i];
      delete [] magArray[i];
    }
    delete [] imageArray; 
    delete [] thetas;
    delete [] magArray;

    return 0;

  }
  
//*******************************
//hysteresis noise filter makes lines continuous and filters out the noise
// see the pdf that we used to understand this step in english (Step 5)
//*******************************
  void hysteresis(void){
    bool greaterFound;
    bool betweenFound;
        for( int j=2 ; j < ROWS-2 ; j++){
	   for( int i=2 ; i < COLUMNS-2 ; i++){
	       if(magArray[i][j] < T_LOW){
		    imageArray[i][j] = 255; // white
	       }
			
	       if(magArray[i][j] > T_HIGH){
		    imageArray[i][j] = 0; // black
	       }
		
	       /*If pixel (x, y) has gradient magnitude between tlow and thigh and 
		 any of its neighbors in a 3 × 3 region around
		 it have gradient magnitudes greater than thigh, keep the edge*/
	       
	       if(magArray[i][j] >= T_LOW && magArray[i][j] <= T_HIGH){
		 greaterFound = false;
		 betweenFound = false;
		 for(int m = -1; m < 2; m++) {
		   for(int n = -1; n < 2; n++){
		     if(magArray[i+m][j+n] > T_HIGH) { 
		       imageArray[i][j] = 0;
		       greaterFound = true;
		     }	     
		     if(magArray[i][j] > T_LOW && magArray[i][j] < T_HIGH) betweenFound = true;
		   }
		 }
		 
		 if(!greaterFound && betweenFound) {
		   for(int m = -2; m < 3; m++) {
		     for(int n = -2; n < 3; n++) {
		       if(magArray[i+m][j+n] > T_HIGH) greaterFound = true;
		     }
		   }
		 }
		 
		 if(greaterFound) imageArray[i][j] = 0;
		 else imageArray[i][j] = 255;
		 
	       }
	       
	     }
	 }
  }	
			
			
/*If pixel (x, y) has gradient magnitude between tlow and thigh and any of its neighbors in a 3 × 3 region around
it have gradient magnitudes greater than thigh, keep the edge (write out white).
• If none of pixel (x, y)’s neighbors have high gradient magnitudes but at least one falls between tlow and thigh,
search the 5 × 5 region to see if any of these pixels have a magnitude greater than thigh. If so, keep the edge
(write out white).
*/
			
  
