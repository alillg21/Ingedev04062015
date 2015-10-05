/** \file
 * Unitary non UTA test case for Displaying a scrolling text box
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/bkw/tcbkw0011.c $
 *
 * $Id: tcbkw0011.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include "SDK30.H"
#include "tst.h"


#ifdef __cplusplus
extern "C"
{
#endif




//******************************************************************************
/*! @brief This function copies a file
 *
 * @param dstFilename : full path to destination file
 * @param srcFilename : full path to source file
 *
 * @return
 * - FILE_OK if file is copied
 * - FILE_ERR_COPY_FAIL if file not copied
 *
 * @link KSFAM_FFMS Back to top @endlink
 */
bool FileCopy(char* dstFileName, char* srcFileName) {
	S_FS_FILE *src = NULL;
	S_FS_FILE *dst = NULL;
	int bytesIn;
	int bytesOut;
	byte buf[4096];

	src = FS_open(srcFileName, "r");
	CHECK(src != NULL, lblKO);

	dst = FS_open(dstFileName, "a");
	CHECK(dst != NULL, lblKO);

	while (TRUE) {
		bytesIn = FS_read(buf, 1, sizeof(buf), src);
		if (bytesIn <= 0)
			break;
		bytesOut = FS_write(buf, 1, bytesIn, dst);
		if (bytesOut < sizeof(buf))
			break;
	}
	FS_close(src);
	FS_close(dst);
	return TRUE;

	lblKO: if (src != NULL)
		FS_close(src);
	if (dst != NULL)
		FS_close(dst);
	return FALSE;
}

#define RECORDSIZE  	512
#define NAMSIZ      	100
#define TUNMLEN      	32
#define TGNMLEN      	32

typedef union TAR_HEADER {
	char charptr[RECORDSIZE];
	struct {
		char name[NAMSIZ]; /* This I want... */
		char mode[8];
		char uid[8];
		char gid[8];
		char size[12]; /* ...and this. The rest can go whistle. */
		char mtime[12];
		char chksum[8];
		char linkflag;
		char linkname[NAMSIZ];
		char magic[8]; /* not much use in practice :-( */
		char uname[TUNMLEN];
		char gname[TGNMLEN];
		char devmajor[8];
		char devminor[8];
	} header;
} TAR_HEADER;

/** Extract file name from a string containing path and file name
 * \param filepath (I) A String containing the full path
 * \return
 *  pointer to string containing the file name
 * \sa Header: tcbkw040.h
 * \sa Source: tcbkw040.c
*/
char * extractFileName(char * filepath)
{
	char * ptr;

	ptr = strrchr(filepath, '\\');

	if (ptr)
		return extractFileName(ptr+1);

	ptr == strrchr(filepath, '/');

	if (ptr)
		return extractFileName(ptr+1);

	return filepath;
}

/** Unpack a PGN file to a destination folder
 * \param destpath (I) Path to destination folder
 * \param pgn_file (I) Path to pgn file
 * \sa Header: tcbkw040.h
 * \sa Source: tcbkw040.c
*/
void Unpack2(char * destpath, char * pgn_file) {
	int ret;
	S_FS_FILE * fp;
	S_FS_FILE * fp1;
	int totalread = 0;
	bool done = FALSE;

	fp = FS_open(pgn_file, "r");
	CHECK(fp != NULL, lblKO);

	ret = FS_seek(fp, 224, FS_SEEKSET);  // skip PGN header
	CHECK(ret == FS_OK, lblKO);

	while (!done)
	{
		TAR_HEADER header;

		int filesize;
		int readsize;
		int len1, len2;
		char buf[256];
		char filename[256];

		memset(buf, 0, sizeof(buf));
		memset(filename, 0, sizeof(filename));

		ret = FS_read(&header, sizeof(header), 1, fp);  // Read file header
		CHECK(ret == 1, lblKO);

		totalread += sizeof(header);

		filesize = strtoul(header.header.size, 0, 8);

		// Check end of file
		if (filesize == 0)
			break;

		if (strlen(extractFileName(header.header.name)) > 15)
		{
			// Skip files with name too long
			FS_seek(fp, filesize, FS_SEEKCUR);
			totalread += filesize;

			FS_seek(fp, RECORDSIZE - totalread % RECORDSIZE, FS_SEEKCUR);
			totalread += RECORDSIZE - totalread % RECORDSIZE;
		}
		else
		{
			strcpy(filename, destpath);
			strcat(filename, "/");
			strcat(filename, extractFileName(header.header.name));

			FS_unlink(filename);

			fp1 = FS_open(filename, "a");
			CHECK(fp1 != NULL, lblKO);

			readsize = 0;
			len1 = 0;
			len2 = 0;
			while (readsize < filesize) {
				if (filesize - readsize > 256)
				{
					len1 = FS_read(buf, 1, 256, fp);
				}
				else
				{
					len1 = FS_read(buf, 1, filesize - readsize, fp);
				}
				len2 = FS_write(buf, 1, len1, fp1);

				CHECK( len1 == len2, lblKO);

				readsize += len1;
				totalread += len1;
			}

			FS_close(fp1);

            // Skip to the start of next 512 block
            if (totalread % RECORDSIZE != 0)
            {
            	FS_seek(fp, RECORDSIZE - totalread % RECORDSIZE, FS_SEEKCUR);
            	totalread += RECORDSIZE - totalread % RECORDSIZE;
            }
		}
	}

lblKO:
	if (fp1)
		FS_close(fp1);
	if (fp)
		FS_close(fp);

}

/** Test parameter file signature check and unpacking
 * \sa Header: tcbkw040.h
 * \sa Source: tcbkw040.c
*/
void tcbkw040(void) {
	S_FS_PARAM_CREATE ParamCreate;

	char RamName[] = "TESTFOLD";
	unsigned int AccessRam = FS_WRITEMANY;
	char mountRam[] = "/TESTFOLD";

	char DestName[] = "DEST";
	unsigned int AccessDest = FS_WRITEONCE;
	char mountDest[] = "/DEST";

	char mountHost[] = "/HOST";
	unsigned int AccessHost = FS_WRITEMANY;

	int retCode;
	unsigned long size;


	strcpy(ParamCreate.Label, RamName);
	ParamCreate.Mode = AccessRam;
	ParamCreate.AccessMode = FS_RONLYMOD;
	ParamCreate.NbFichierMax = 100;
	ParamCreate.IdentZone = FS_WO_ZONE_DATA;

	size = 1048576 * 8;
	retCode = FS_dskkill(mountRam);
	retCode = FS_dskcreate(&ParamCreate, &size);


	strcpy(ParamCreate.Label, DestName);
	ParamCreate.Mode = AccessDest;
	ParamCreate.AccessMode = FS_RONLYMOD;
	ParamCreate.NbFichierMax = 100;
	ParamCreate.IdentZone = FS_WO_ZONE_DATA;

	size = 1048576 * 10;
	retCode = FS_dskkill(mountDest);
	retCode = FS_dskcreate(&ParamCreate, &size);

	retCode = FS_mount(mountRam, &AccessRam);
	retCode = FS_mount(mountHost, &AccessHost);
	retCode = FS_mount(mountDest, &AccessDest);

	// Copy PGN file to RAM disk
	retCode = FileCopy("/TESTFOLD/DATA015D.PGN", "/HOST/DATA015D.PGN");

	// Check signature and parameters
	retCode = NTP_SoftwareCheck(mountRam);

	// Unpack to Dest
	if (retCode == 0) {
		Unpack2(mountDest, "/TESTFOLD/DATA015D.PGN");
	}

	FS_unmount(mountHost);
	FS_unmount(mountRam);
	FS_unmount(mountDest);
}

#ifdef __cplusplus
}
#endif
