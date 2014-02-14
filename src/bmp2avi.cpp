
#include <windows.h>
#include <vfw.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int interlace = 0;
int deleteAfter = 0;
int framesPerSecond = 25;
const char *aviFileName = "out.avi";
const char *szPrefix = NULL;
int nbImages = 0;
int dx, dy;
int firstImg = 0;

PAVIFILE aviFile;
PAVISTREAM pstream;

static void err_exit( const char *fmt, ... )
{
	char szBuff[2048];
	va_list ap;

	va_start( ap, fmt );
	vsprintf( szBuff, fmt,ap );
	strcat( szBuff, "\n" );
	fputs( szBuff, stderr );
	va_end( ap );
	exit( 1 );
}

static void WorkInProgess(void)
{
	printf( "." );
	fflush( stdout );
}

static void create_avi_file( const char *name, int nbImages, int dx, int dy )
{
	LONG hr;
	AVISTREAMINFO info;

	AVIFileInit();
	hr = AVIFileOpen( &aviFile, name, OF_CREATE | OF_WRITE | OF_SHARE_DENY_NONE, NULL );
	if( hr != 0 ){
		err_exit( "Unable to open file '%s'.", name );
	}

	info.fccType = streamtypeVIDEO;
	info.fccHandler = mmioFOURCC( 'D', 'I', 'B', ' ' );
	info.dwFlags = 0;
	info.dwCaps = 0;
	info.wPriority = 0;
	info.wLanguage = 0;
	info.dwScale = 1;	// An idea like suspicion
	info.dwRate = framesPerSecond;
	info.dwStart = 0;	//	ALWAYS
	info.dwLength = nbImages;
	info.dwInitialFrames = 0; // NO SOUND in it
	info.dwSuggestedBufferSize = 0; // ADOBE PREMIERE will choose!
	info.dwQuality = (DWORD)-1; // Default value (good choice?)
	info.dwSampleSize = 0; // For video stream
	info.rcFrame.left = info.rcFrame.top = 0;
	info.rcFrame.right = dx;
	info.rcFrame.bottom = dy;
	info.dwEditCount = 0;
	info.dwFormatChangeCount = 0;
	strncpy( info.szName, "Direct BITMAP Conversion", sizeof(info.szName) );

	if( hr = AVIFileCreateStream( aviFile, &pstream, &info ) ){
		err_exit( "Can not create AVI stream." );
	}
	if( hr = AVIStreamRelease( pstream ) ){
		err_exit( "Can not close AVI stream." );
	}
}

static const char *getArg( char *argv[], int *arg )
{
	const char *ret;
	if( argv[ *arg ][2] == '\0' ){
		(*arg)++;
		if( !argv[ *arg ] || !(argv[ *arg ][0]) ){
			err_exit( "Bad argument for last option." );
		}
		else {
			ret = argv[ *arg ];
		}
	}
	else {
		ret = &argv[ *arg ][2];
	}
	return ret;
}

static void check_bitmap( const char *szName, FILE *f, int *dx, int *dy )
{
	BITMAPFILEHEADER header;
	BITMAPINFOHEADER info;

	if( fseek( f, 0, SEEK_SET ) != 0 ){
		err_exit( "Can not seek begin of file '%s'", szName );
	}

	if( !fread( &header, sizeof( BITMAPFILEHEADER ), 1, f ) ){
		err_exit( "Can not read HEADER of file '%s'", szName );
	}

	if( strncmp( (const char *)&header.bfType, "BM", 2 ) ){
		err_exit( "Bad file type for '%s' (not Windows BITMAP)", szName );
	}

	/*	Other information	*/
	if( !fread( &info, sizeof( BITMAPINFOHEADER ), 1, f ) ){
		err_exit( "Can not read INFORMATION of file '%s'", szName );
	}

	/*	Get size of the image	*/
	*dx = (int)info.biWidth;
	*dy = (int)info.biHeight;

	if( info.biCompression != BI_RGB ){
		err_exit( "This program does not understand compressed files." );
	}

	/*	Set file pointer to start of data */
	fseek( f, header.bfOffBits, SEEK_SET );
}


static void delete_file( const char *prefix, int num )
{
	int n;
	char szBuff[260];
	FILE *f;

	/*	Rewrite-it, very bad programming...	*/
	for( n = 1; n < 5; n++ ){
		sprintf( szBuff, "%s%0*d.bmp", prefix, n, num + firstImg );
		f = fopen( szBuff, "rb" );
		if( f != NULL ) {
			fclose( f );
			if( remove( szBuff ) ){
				fprintf( stderr, "Can not delete frame #%d. Error %d\n", num, errno );
				return;
			}
		}
	}
	fprintf( stderr, "Can not find frame #%d!!!\n", num );
}

/*
 *	This is a very rubbish code...
 *	What are you doing? Not already corrected!
 *
 */

static FILE * open_file( const char *prefix, int num )
{
	FILE *f;
	int n;
	char szBuff[ 260 ];

	for( n = 1; n < 5; n++ ){
		sprintf( szBuff, "%s%0*d.bmp", prefix, n, num + firstImg );
		f = fopen( szBuff, "rb" );
		if( f != NULL ) return f;
	}
	return NULL;
}


static int exist_file( const char *prefix, int num, int *dx, int *dy )
{
	FILE *f;

	f = open_file( prefix, num );
	if( f != NULL ){
		char szBuff[ 256 ];
		sprintf( szBuff, "FILE #%d", num + firstImg );
		check_bitmap( szBuff, f, dx, dy );
		fclose( f );
		return 1;
	}
	return 0;
}

/*
 *	So simple! A little stupid function...
 */
static int compute_nb_files( const char *szPrefix, int *dx, int *dy )
{
	int dxmem, dymem;
	int nb = 0;

	while( exist_file( szPrefix, nb, dx, dy ) ){
		if( nb == 0 ){
			dxmem = *dx;
			dymem = *dy;
		}
		else if( *dx != dxmem || *dy != dymem ){
			err_exit( "BITMAP #%d not same size.", nb );
		}
		nb++;
	}
	return nb;
}

static int getIntArg(char *argv[], int *arg )
{
	int v = 0;
	const char *data = getArg( argv, arg );

	while( isdigit( *data ) ){
		v = v * 10 + (*data - '0');
		data++;
	}
	if( *data != '\0' ){
		err_exit( "Bad integer value. Check your command line." );
	}
	return v;
}

LPBITMAPINFOHEADER getBitmap( FILE *f, int frame, int *fsize, int *s )
{
	LPBITMAPINFOHEADER pbmih = NULL;
	
	if( fseek( f, 0, SEEK_END ) != 0 ){
		err_exit( "Can not seek at the end of frame #%d!", frame );
	}
	if( (*fsize = ftell( f )) == -1 ){
		err_exit( "Can not seek at the end of frame #%d!", frame );
	}
	if( fseek( f, sizeof( BITMAPFILEHEADER ), SEEK_SET ) != 0 ){
		err_exit( "Size of BITMAPFILEHEADER not correct for frame #%d.", frame );
	}

	*fsize -= sizeof( BITMAPFILEHEADER );
	pbmih = (LPBITMAPINFOHEADER)malloc( *fsize );
	if( fread( pbmih, *fsize, 1, f ) != 1 ){
		err_exit( "Can not read BITMAP data." );
	}

	if( pbmih->biClrUsed == 0 ){
		switch( pbmih->biBitCount ){
			case 1 : *s = 8; break;
			case 4 : *s = 64; break;
			case 8 : *s = 1024; break;
			case 24 : *s = 0; break;
			case 16 :
			case 32 : 
				*s = pbmih->biCompression == BI_RGB ? 12 : 0;
				break;
			default :
				err_exit( "Oups! BUG IS HERE?!?" );
		}
	}
	else {
		*s = pbmih->biClrUsed * 4;
	}
	*s += sizeof( BITMAPINFOHEADER );

	return pbmih;
}

/*
**
**
*/
void writeBitmap( FILE *f, int frame )
{
	LPBITMAPINFOHEADER pbmih = NULL;
	int s, fsize;

	pbmih = getBitmap( f, frame, &fsize, &s );
	if( AVIStreamSetFormat( pstream, frame, pbmih, s ) != 0 ){
		err_exit( "Can not set format to AVI file." );
	}
	if( AVIStreamWrite( pstream, frame, 1, (LPBYTE)pbmih+s,
				fsize-s, AVIIF_KEYFRAME, NULL, NULL ) != 0 ){
		err_exit( "Can not write to AVI file." );
	}

	if( pbmih ) free( pbmih );
}

void writeInterlace( FILE *f1, FILE *f2, int frame )
{
	int pixel_length;
	int i;
	LPBITMAPINFOHEADER pbmih[2];
	int s[2], fsize[2];

	for( i = 0; i < 2; i++ ){
		pbmih[i] = getBitmap( (i==0? f1 : f2), frame, &fsize[i], &s[i] );
	}
	if( s[0] != s[1] || fsize[0] != fsize[1] ){
		err_exit( "Something differ from two bitmaps for frame #%d", frame );
	}

	/*	All right... Now the "real" work!!!
	**	I LOVE IT: the following lines are the only
	**	lines I really need (surprising, no?)
	**	Just for these dozen of lines... what work!
	*/

	/*	Copy lines!!! Trvial, no? */
	pixel_length = pbmih[0]->biBitCount / 8;
	for( i = 0; i < dy; i += 2 ){
		LPBYTE line[2];
		int j;
		for( j=0; j < 2; j++ ){
			line[j] = (LPBYTE)pbmih[j] + s[j];
			line[j] += ((i+j) * (pixel_length * dx));
		}
		memcpy( line[0], line[1], (pixel_length * dx) );
	}

	/*
	**	All right... Now OK - Classic!
	*/
	if( AVIStreamSetFormat( pstream, frame, pbmih[0], s[0] ) != 0 ){
		err_exit( "Can not set format to AVI interlace file." );
	}
	if( AVIStreamWrite( pstream, frame, 1, (LPBYTE)pbmih[0]+s[0],
				fsize[0]-s[0], AVIIF_KEYFRAME, NULL, NULL ) != 0 ){
		err_exit( "Can not write to AVI interlace file." );
	}
	printf( "." );
	fflush( stdout );

	for(i = 0; i<2; i++){
		if( pbmih[i] ) free( pbmih[i] );
	}
}

void usage(const char *prgName)
{
	printf( "Usage: %s [options] <rootname>\n", prgName );
	printf( "\n"
			"-I interlace is on. Odd lines first\n"
			"-i interlace is on. Even lines first\n"
			"-f frames by second (25 by default)\n"
			"-d delete files after include (use with care)\n"
			"-o output filename (OUT.AVI if not given)\n"
			"-s first image indice (if not zero)\n"
			"-v display version of program and exits.\n"
			"rootname: the root name for the .BMP files.\n"
			"\n" );
	exit( 0 );
}

int avimain( int argc, char *argv[] )
{
	int i;

	for( i = 1; i < argc; i++ ){
		if( argv[i][0] == '-' ){
			switch( argv[i][1] ){
				case 'i' :
					interlace = -1;
					break;

				case 'I' :
					interlace = +1;
					break;

				case 'd' :
					deleteAfter = 1;
					break;

				case 'f' :
					framesPerSecond = getIntArg( argv, &i );
					break;

				case 'o' :
					aviFileName = getArg( argv, &i );
					break;

				case 's' :
					firstImg = atoi( getArg( argv, &i ) );
					break;

				case 'v' :
					fprintf( stdout, "Version 1.01\n"
						"New releases at http://willsoft.free.fr/\n" );
					exit( 0 );
					break;

				case '?' :
					usage(argv[0]);
					break;

				default :
					err_exit( "Unknown option '%c'", argv[i][1] );
			}
		}
		else {
			if( szPrefix ){
				err_exit( "Prefix for BMP files already specified." );
			}
			szPrefix = argv[i];
		}
	}

	/*	Looking for files	*/
	nbImages = compute_nb_files( szPrefix, &dx, &dy );
	if( nbImages == 0 ){
		err_exit( "First image must be at index 0." );
	}
	else if( nbImages < 5 ) {
		/* OK: it is not an good excuse... */
		err_exit( "This program needs more than 4 images to run." );
	}

	if( interlace != 0 ){
		if( nbImages % 2 != 0){
			fprintf( stderr, "Only %d images will be used.\n", nbImages / 2 );
		}
		if( dy % 2 != 0 ){
			err_exit( "A interlace video needs a pair numbre of lines in BITMAPs." );
		}
		nbImages /= 2; // So, interlace...
	}

	/*	We are sure of file given.
	**
	**	If you like jokes, you can erase, modify
	**	or change files during the transfert to
	**	AVI format. Do not complain about that. I
	**	know it, but not my problem!
	*/

	create_avi_file( aviFileName, nbImages, dx, dy );
	
	/*	Now, work can start	*/
	for( i = 0; i < nbImages; i++ ){
		if( interlace == 0 ){
			FILE *f;
			f = open_file( szPrefix, i );
			if( f == NULL ) err_exit( "Can not open image #%d. Are you joking?", i );
			WorkInProgess();
			writeBitmap( f, i );
			fclose( f );
			if( deleteAfter ) delete_file( szPrefix, i );
		}
		else {
			FILE *f1, *f2;
			if( interlace == -1 ){
				f1 = open_file( szPrefix, i * 2 + 0 );
				f2 = open_file( szPrefix, i * 2 + 1 );
			}
			else {
				/*	Just inverse files!	*/
				f1 = open_file( szPrefix, i * 2 + 1 );
				f2 = open_file( szPrefix, i * 2 + 0 );
			}
			if( f1 == NULL || f2 == NULL) err_exit( "Can not open image #%d. Are you joking?", i );
			writeInterlace( f1, f2, i );
			fclose( f1 );
			fclose( f2 );
			if( deleteAfter ) {
				delete_file( szPrefix, i * 2 );
				delete_file( szPrefix, i * 2 + 1 );
			}
		}
	}

	AVIFileRelease( aviFile );
	AVIFileExit();
	return 0;
}
