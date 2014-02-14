// -------------------------------------------------------------
//
// FILE: VolVis_IO
//
// INPUT/OUTPUT routines for VolVis format volume files
//
// -------------------------------------------------------------

#include "sysconf.h"
#include "VolVis_IO.h"

using namespace std;

const char * TypeStr::ProjectionStyle[]  = PROJECTION_STYLES_STR;
const char * TypeStr::Unit[]             = UNIT_TYPE_STR;
const char * TypeStr::DataOrigin[]       = DATA_ORIGIN_TYPE_STR;
const char * TypeStr::DataModification[] = DATA_MODIFICATION_TYPE_STR;
const char * TypeStr::DataCompression[]  = DATA_COMPRESSION_TYPE_STR;

// -------------------------------------------------------------
//  LoadVolVisDataset
//    Read a volume dataset from a VolVis
//    format file.
// -------------------------------------------------------------
unsigned char * LoadVolVisDataset(const char* datasetFileName, unsigned int* x,
                                  unsigned int* y, unsigned int* z)
{
  // load dataset temporarely into regular unskewed grid array
  // then hand that array to the memory class for permanent
  // (and skewed) storage
 
  int magicDataTypeNumber;
  int bitsPerVoxel;
  float unitLengthX, unitLengthY, unitLengthZ;
  int unitType, dataOrigin, dataModification, compressionType;
  unsigned char* voxelData;
  int datasetSize;
  int dataSliceSize;
  int compressedSize;
  unsigned char *compressedSlice;
  unsigned char *slice;
  unsigned char X;
 
  cout << endl << "loading dataset \""<< datasetFileName << "\"" << endl;
#ifdef WIN32
  ifstream srcFile(datasetFileName, ios::binary);
#else
  ifstream srcFile(datasetFileName);
#endif
  srcFile >> magicDataTypeNumber;
  srcFile >> *x >> *y >> *z;
  srcFile >> bitsPerVoxel;
  srcFile >> unitLengthX >> unitLengthY >> unitLengthZ;
  srcFile >> unitType >> dataOrigin >> dataModification >> compressionType;

  cout << "magicDataTypeNumber: " << magicDataTypeNumber << endl;
  cout << "datasetSizeUVW: " << *x << ", " << *y << ", " << *z << endl;
  cout << "bitsPerVoxel: " << bitsPerVoxel << endl;
  cout << "unitLength: x=" << unitLengthX << endl;
  cout << "            y=" << unitLengthY << endl;
  cout << "            z=" << unitLengthZ << endl;
  //cout << "unitType: " << TypeStr::Unit[unitType] << endl;
  //cout << "dataOrigin: " << TypeStr::DataOrigin[dataOrigin] << endl;
  //cout << "dataModification: " << 
  //  TypeStr::DataModification[dataModification] << endl;
  //cout << "compressionType: " <<
  //  TypeStr::DataCompression[compressionType] << endl;
	
  ReadVolVisIcon(&srcFile);

  // read dataset 
  datasetSize = (*x) * (*y) * (*z);
  dataSliceSize = (*x) * (*y);

  compressedSlice = new unsigned char[2*dataSliceSize];
  //assert(compressedSlice);

  voxelData = new unsigned char[datasetSize];
  //assert(voxelData);

  slice = voxelData;
 
  for (unsigned int i=0; i<(*z); ++i, slice += dataSliceSize) {
    switch (compressionType) {
    case NO_COMPRESSION:
      srcFile.read((char*)slice,dataSliceSize);
      break;
    case RUN_LENGTH_ENCODE:
      // read compressed slice size
      srcFile >> compressedSize;

      // advance to real data
      for ( X = '\0';  X != 'X'; srcFile.get((char&)X) ) ;

      // read in slice
      srcFile.read((char*)compressedSlice, compressedSize);
      //assert( compressedSize == (int) srcFile.gcount() );

      // decompress slice
      // see also:
      //     /home/fs2/mmshare/pkg/VolVis.2.1/lib/file_io/src/C_compression.c
      register unsigned char *compressData = compressedSlice;
      register unsigned char *voxel = slice;
      register unsigned char currentValue = *compressData;
      register unsigned char remaining;

      for (currentValue = *(compressData++);
           (remaining = (currentValue & 0x7f));
	   currentValue = *(compressData++) ) {
	if ( currentValue & 0x80 ) {
	  while ( remaining-- )
	    *(voxel++) = *(compressData++);
	}
	else {
	  currentValue = *(compressData++);
	  while ( remaining-- )
	    *(voxel++) = currentValue;
	}
      }
      break;
    } // switch
 
  } // for i
  delete compressedSlice; // free memory
 
  return voxelData;
} // LoadVolVisDataset


// -------------------------------------------------------------
//  LoadVolVisDataset
//    Read a volume dataset from a VolVis
//    format file.
// -------------------------------------------------------------
unsigned char * LoadVolVisDataset(const char* datasetFileName, int* x,
                                  int* y, int*z,
                                  float* xScl, float* yScl, float* zScl)
{
  // load dataset temporarely into regular unskewed grid array
  // then hand that array to the memory class for permanent
  // (and skewed) storage
 
  int magicDataTypeNumber;
  int bitsPerVoxel;
  float unitLengthX, unitLengthY, unitLengthZ;
  int unitType, dataOrigin, dataModification, compressionType;
  unsigned char* voxelData;
  int datasetSize;
  int dataSliceSize;
  int compressedSize;
  unsigned char *compressedSlice;
  unsigned char *slice;
  unsigned char X;
 
  cout << endl << "loading dataset \""<< datasetFileName << "\"" << endl;
  ifstream srcFile(datasetFileName, ios::binary);
  srcFile >> magicDataTypeNumber;
  srcFile >> *x >> *y >> *z;
  srcFile >> bitsPerVoxel;
  srcFile >> unitLengthX >> unitLengthY >> unitLengthZ;
  srcFile >> unitType >> dataOrigin >> dataModification >> compressionType;

  *xScl = unitLengthX;
  *yScl = unitLengthY;
  *zScl = unitLengthZ;

  cout << "magicDataTypeNumber: " << magicDataTypeNumber << endl;
  cout << "datasetSizeUVW: " << *x << ", " << *y << ", " << *z << endl;
  cout << "bitsPerVoxel: " << bitsPerVoxel << endl;
  cout << "unitLength: x=" << unitLengthX << endl;
  cout << "            y=" << unitLengthY << endl;
  cout << "            z=" << unitLengthZ << endl;
  cout << "unitType: " << TypeStr::Unit[unitType] << endl;
  cout << "dataOrigin: " << TypeStr::DataOrigin[dataOrigin] << endl;
  cout << "dataModification: " << 
    TypeStr::DataModification[dataModification] << endl;
  cout << "compressionType: " <<
    TypeStr::DataCompression[compressionType] << endl;
	
  ReadVolVisIcon(&srcFile);

  // read dataset 
  datasetSize = (*x) * (*y) * (*z);
  dataSliceSize = (*x) * (*y);

  compressedSlice = new unsigned char[2*dataSliceSize];
  //assert(compressedSlice);

  voxelData = new unsigned char[datasetSize];
  //assert(voxelData);

  slice = voxelData;
 
  for (int i=0; i<(*z); ++i, slice += dataSliceSize) {
    switch (compressionType) {
    case NO_COMPRESSION:
      srcFile.read((char*)slice,dataSliceSize);
      break;
    case RUN_LENGTH_ENCODE:
      // read compressed slice size
      srcFile >> compressedSize;

      // advance to real data
      for ( X = '\0';  X != 'X'; srcFile.get((char&)X) ) ;

      // read in slice
      srcFile.read((char*)compressedSlice, compressedSize);
      //assert( compressedSize == (int) srcFile.gcount() );

      // decompress slice
      // see also:
      //     /home/fs2/mmshare/pkg/VolVis.2.1/lib/file_io/src/C_compression.c
      register unsigned char *compressData = compressedSlice;
      register unsigned char *voxel = slice;
      register unsigned char currentValue = *compressData;
      register unsigned char remaining;

      for (currentValue = *(compressData++);
           (remaining = (currentValue & 0x7f));
	   currentValue = *(compressData++) ) {
	if ( currentValue & 0x80 ) {
	  while ( remaining-- )
	    *(voxel++) = *(compressData++);
	}
	else {
	  currentValue = *(compressData++);
	  while ( remaining-- )
	    *(voxel++) = currentValue;
	}
      }
      break;
    } // switch
 
  } // for i
  delete compressedSlice; // free memory
 
  return voxelData;
} // LoadVolVisDataset


// -------------------------------------------------------------
//  LoadVolVisTexture
//    Read a texture dataset from a VolVis
//    format file.
// -------------------------------------------------------------
unsigned char * LoadVolVisTexture(const char* datasetFileName, int* x,
                                  int* y, int*z)
{
  // load dataset temporarely into regular unskewed grid array
  // then hand that array to the memory class for permanent
  // (and skewed) storage
 
  int magicDataTypeNumber;
  int textureType;
  unsigned char* texelData;
 
  cout << endl << "loading texture \""<< datasetFileName << "\"" << endl;
#ifdef WIN32
  ifstream srcFile(datasetFileName, ios::binary);
#else
  ifstream srcFile(datasetFileName);
#endif
  srcFile >> magicDataTypeNumber;
  srcFile >> textureType;
  srcFile >> *x >> *y >> *z;

  cout << "magicDataTypeNumber: " << magicDataTypeNumber << endl;
  cout << "textureType: " << textureType << endl;
  cout << "datasetSizeUVW: " << *x << ", " << *y << ", " << *z << endl;
	
  ReadVolVisIcon(&srcFile);

  // read dataset (RGB)
  texelData = new unsigned char[(*x) * (*y) * (*z) * 3];

  srcFile.read((char*)texelData,(*x) * (*y) * (*z) * 3);
 
  return texelData;
} // LoadVolVisTexture


// -------------------------------------------------------------
//  ReadVolVisIcon
//    Read the ICON from a VolVis format file.
// -------------------------------------------------------------
void ReadVolVisIcon(ifstream* srcFile)
{
  unsigned char *iconRed, *iconGreen, *iconBlue;
  unsigned char X;
  int iconX, iconY;
 
  (*srcFile) >> iconX >> iconY;

  cout << "Icon: " << iconX << ", " << iconY << endl;
 
  for ( X = '\0';  X != 'X'; (*srcFile).get((char&)X) );

  // load icon
  iconRed   = new unsigned char[iconX * iconY];
  //assert(iconRed);
  iconGreen = new unsigned char[iconX * iconY];
  //assert(iconGreen);
  iconBlue  = new unsigned char[iconX * iconY];
  //assert(iconBlue);
 
  (*srcFile).read((char*)iconRed,   iconX*iconY);
  (*srcFile).read((char*)iconGreen, iconX*iconY);
  (*srcFile).read((char*)iconBlue,  iconX*iconY);

#ifdef SHOW_ICON
  // save icon as seperate rgb file
  ofstream iconFile("icon.rgb");
  iconFile.write(iconRed,   iconX*iconY);
  iconFile.write(iconGreen, iconX*iconY);
  iconFile.write(iconBlue,  iconX*iconY);
#endif
 
  delete iconRed;
  delete iconGreen;
  delete iconBlue;
 
} // ReadVolVisIcon

