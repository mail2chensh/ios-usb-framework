/*
	DOSFS Embedded FAT-Compatible Filesystem
	(C) 2005 Lewin A.R.W. Edwards (sysadm@zws.com)

	You are permitted to modify and/or use this code in your own projects without
	payment of royalty, regardless of the license(s) you choose for those projects.

	You cannot re-copyright or restrict use of the code as released by Lewin Edwards.
*/

#include <string.h>
#include <stdlib.h>
    
#include "dosfs.h"
    
/*
	Get starting sector# of specified partition on drive #unit
	NOTE: This code ASSUMES an MBR on the disk.
	scratchsector should point to a SECTOR_SIZE scratch area
	Returns 0xffffffff for any error.
	If pactive is non-NULL, this function also returns the partition active flag.
	If pptype is non-NULL, this function also returns the partition type.
	If psize is non-NULL, this function also returns the partition size.
*/ 
    uint32_t DFS_GetPtnStart(uint8_t unit, uint8_t * scratchsector,
                             uint8_t pnum, uint8_t * pactive, uint8_t * pptype,
                             uint32_t * psize) 
{
        
        
        
            // DOS ptable supports maximum 4 partitions
            if (pnum > 3)
                
        
            // Read MBR from target media
            if (DFS_ReadSector(unit, scratchsector, 0, 1)) {
                
        
        
            (((uint32_t) mbr->ptable[pnum].start_1) << 8) | 
            (((uint32_t) mbr->ptable[pnum].start_2) << 16) | 
            (((uint32_t) mbr->ptable[pnum].start_3) << 24);
        
                
        
                
        
                
                    (((uint32_t) mbr->ptable[pnum].size_1) << 8) | 
                    (((uint32_t) mbr->ptable[pnum].size_2) << 16) | 
                    (((uint32_t) mbr->ptable[pnum].size_3) << 24);
        



/*
	Retrieve volume info from BPB and store it in a VOLINFO structure
	You must provide the unit and starting sector of the filesystem, and
	a pointer to a sector buffer for scratch
	Attempts to read BPB and glean information about the FS from that.
	Returns 0 OK, nonzero for any error.
*/ 
    uint32_t DFS_GetVolInfo(uint8_t unit, uint8_t * scratchsector,
                            uint32_t startsector, PVOLINFO volinfo) 
{
        
        
        
        
                
        
// tag: OEMID, refer dosfs.h
//      strncpy(volinfo->oemid, lbr->oemid, 8);
//      volinfo->oemid[8] = 0;
            
        
            (((uint16_t) lbr->bpb.reserved_h) << 8);
        
            (((uint16_t) lbr->bpb.sectors_s_h) << 8);
        
                
                    (((uint32_t) lbr->bpb.sectors_l_1) << 8) | 
                    (((uint32_t) lbr->bpb.sectors_l_2) << 16) | 
                    (((uint32_t) lbr->bpb.sectors_l_3) << 24);
        
            // If secperfat is 0, we must be in a FAT32 volume; get secperfat
            // from the FAT32 EBPB. The volume label and system ID string are also
            // in different locations for FAT12/16 vs FAT32.
            volinfo->secperfat = (uint16_t) lbr->bpb.secperfat_l | 
            (((uint16_t) lbr->bpb.secperfat_h) << 8);
        
                
                    (((uint32_t) lbr->ebpb.ebpb32.fatsize_1) << 8) | 
                    (((uint32_t) lbr->ebpb.ebpb32.fatsize_2) << 16) | 
                    (((uint32_t) lbr->ebpb.ebpb32.fatsize_3) << 24);
                
                
                
// tag: OEMID, refer dosfs.h
//              memcpy(volinfo->system, lbr->ebpb.ebpb32.system, 8);
//              volinfo->system[8] = 0; 
        }
        
        else {
                
                
                
// tag: OEMID, refer dosfs.h
//              memcpy(volinfo->system, lbr->ebpb.ebpb.system, 8);
//              volinfo->system[8] = 0; 
        }
        
            // note: if rootentries is 0, we must be in a FAT32 volume.
            volinfo->rootentries = (uint16_t) lbr->bpb.rootentries_l | 
            (((uint16_t) lbr->bpb.rootentries_h) << 8);
        
            // after extracting raw info we perform some useful precalculations
            volinfo->fat1 = startsector + volinfo->reservedsecs;
        
            // The calculation below is designed to round up the root directory size for FAT12/16
            // and to simply ignore the root directory for FAT32, since it's a normal, expandable
            // file in that situation.
            if (volinfo->rootentries) {
                
                
                    volinfo->rootdir +
                    (((volinfo->rootentries * 32) +
                      (SECTOR_SIZE - 1)) / SECTOR_SIZE);
        
        
        else {
                
                
                    (((uint32_t) lbr->ebpb.ebpb32.root_1) << 8) | 
                    (((uint32_t) lbr->ebpb.ebpb32.root_2) << 16) | 
                    (((uint32_t) lbr->ebpb.ebpb32.root_3) << 24);
        
        
            // Calculate number of clusters in data area and infer FAT type from this information.
            volinfo->numclusters =
            (volinfo->numsecs - volinfo->dataarea) / volinfo->secperclus;
        
                
        
        else if (volinfo->numclusters < 65525)
                
        
        else
                
        



/*
	Fetch FAT entry for specified cluster number
	You must provide a scratch buffer for one sector (SECTOR_SIZE) and a populated VOLINFO
	Returns a FAT32 BAD_CLUSTER value for any error, otherwise the contents of the desired
	FAT entry.
	scratchcache should point to a UINT32. This variable caches the physical sector number
	last read into the scratch buffer for performance enhancement reasons.
*/ 
    uint32_t DFS_GetFAT(PVOLINFO volinfo, uint8_t * scratch,
                        uint32_t * scratchcache, uint32_t cluster) 
{
        
        
                
        
        
        else if (volinfo->filesystem == FAT16) {
                
        
        
        else if (volinfo->filesystem == FAT32) {
                
        
        
        else
                
        
            // at this point, offset is the BYTE offset of the desired sector from the start
            // of the FAT. Calculate the physical sector containing this FAT entry.
            sector = ldiv(offset, SECTOR_SIZE).quot + volinfo->fat1;
        
            // If this is not the same sector we last read, then read it into RAM
            if (sector != *scratchcache) {
                
                        
                            // avoid anyone assuming that this cache value is still valid, which
                            // might cause disk corruption
                            *scratchcache = 0;
                        
                }
                
        
        
            // At this point, we "merely" need to extract the relevant entry.
            // This is easy for FAT16 and FAT32, but a royal PITA for FAT12 as a single entry
            // may span a sector boundary. The normal way around this is always to read two
            // FAT sectors, but that luxury is (by design intent) unavailable to DOSFS.
            offset = ldiv(offset, SECTOR_SIZE).rem;
        
                
                    // Special case for sector boundary - Store last byte of current sector.
                    // Then read in the next sector and put the first byte of that sector into
                    // the high byte of result.
                    if (offset == SECTOR_SIZE - 1) {
                        
                        
                        
                                
                                    // avoid anyone assuming that this cache value is still valid, which
                                    // might cause disk corruption
                                    *scratchcache = 0;
                                
                        }
                        
                        
                            // Thanks to Claudio Leonel for pointing out this missing line.
                            result |= ((uint32_t) scratch[0]) << 8;
                
                
                else {
                        
                            ((uint32_t) scratch[offset + 1]) << 8;
                
                
                        
                
                else
                        
        
        
        else if (volinfo->filesystem == FAT16) {
                
                    ((uint32_t) scratch[offset + 1]) << 8;
        
        
        else if (volinfo->filesystem == FAT32) {
                
                           ((uint32_t) scratch[offset + 1]) << 8 | 
                           ((uint32_t) scratch[offset + 2]) << 16 | 
                           ((uint32_t) scratch[offset + 3]) << 24) & 0x0fffffff;
        
        
        else
                
        return result;



/*
	Set FAT entry for specified cluster number
	You must provide a scratch buffer for one sector (SECTOR_SIZE) and a populated VOLINFO
	Returns DFS_ERRMISC for any error, otherwise DFS_OK
	scratchcache should point to a UINT32. This variable caches the physical sector number
	last read into the scratch buffer for performance enhancement reasons.

	NOTE: This code is HIGHLY WRITE-INEFFICIENT, particularly for flash media. Considerable
	performance gains can be realized by caching the sector. However this is difficult to
	achieve on FAT12 without requiring 2 sector buffers of scratch space, and it is a design
	requirement of this code to operate on a single 512-byte scratch.

	If you are operating DOSFS over flash, you are strongly advised to implement a writeback
	cache in your physical I/O driver. This will speed up your code significantly and will
	also conserve power and flash write life.
*/ 
    uint32_t DFS_SetFAT(PVOLINFO volinfo, uint8_t * scratch,
                        uint32_t * scratchcache, uint32_t cluster,
                        uint32_t new_contents) 
{
        
        
                
                
        
        
        else if (volinfo->filesystem == FAT16) {
                
                
        
        
        else if (volinfo->filesystem == FAT32) {
                
                
        }
        
        else
                
        
            // at this point, offset is the BYTE offset of the desired sector from the start
            // of the FAT. Calculate the physical sector containing this FAT entry.
            sector = ldiv(offset, SECTOR_SIZE).quot + volinfo->fat1;
        
            // If this is not the same sector we last read, then read it into RAM
            if (sector != *scratchcache) {
                
                        
                            // avoid anyone assuming that this cache value is still valid, which
                            // might cause disk corruption
                            *scratchcache = 0;
                        
                
                
        
        
            // At this point, we "merely" need to extract the relevant entry.
            // This is easy for FAT16 and FAT32, but a royal PITA for FAT12 as a single entry
            // may span a sector boundary. The normal way around this is always to read two
            // FAT sectors, but that luxury is (by design intent) unavailable to DOSFS.
            offset = ldiv(offset, SECTOR_SIZE).rem;
        
                
                    // If this is an odd cluster, pre-shift the desired new contents 4 bits to
                    // make the calculations below simpler
                    if (cluster & 1)
                        
                
                    // Special case for sector boundary
                    if (offset == SECTOR_SIZE - 1) {
                        
                            // Odd cluster: High 12 bits being set
                            if (cluster & 1) {
                                
                                    (scratch[offset] & 0x0f) | new_contents &
                                    0xf0;
                        
                        
                            // Even cluster: Low 12 bits being set
                            else {
                                
                        
                        
                            DFS_WriteSector(volinfo->unit, scratch,
                                            *scratchcache, 1);
                        
                            // mirror the FAT into copy 2
                            if (DFS_OK == result)
                                
                                    DFS_WriteSector(volinfo->unit, scratch,
                                                    (*scratchcache) +
                                                    volinfo->secperfat, 1);
                        
                            // If we wrote that sector OK, then read in the subsequent sector
                            // and poke the first byte with the remainder of this FAT entry.
                            if (DFS_OK == result) {
                                
                                
                                    DFS_ReadSector(volinfo->unit, scratch,
                                                   *scratchcache, 1);
                                
                                        
                                            // Odd cluster: High 12 bits being set
                                            if (cluster & 1) {
                                                
                                                    new_contents & 0xff00;
                                        
                                        
                                            // Even cluster: Low 12 bits being set
                                            else {
                                                
                                                    (scratch[0] & 0xf0) |
                                                    new_contents & 0x0f;
                                        
                                        
                                            DFS_WriteSector(volinfo->unit,
                                                            scratch,
                                                            *scratchcache, 1);
                                        
                                            // mirror the FAT into copy 2
                                            if (DFS_OK == result)
                                                
                                                    DFS_WriteSector
                                                    (volinfo->unit, scratch,
                                                     (*scratchcache)
                                                     + volinfo->secperfat, 1);
                                
                                
                                else {
                                        
                                            // avoid anyone assuming that this cache value is still valid, which
                                            // might cause disk corruption
                                            *scratchcache = 0;
                                
                        
                
                
                    // Not a sector boundary. But we still have to worry about if it's an odd
                    // or even cluster number.
                    else {
                        
                            // Odd cluster: High 12 bits being set
                            if (cluster & 1) {
                                
                                    (scratch[offset] & 0x0f) | new_contents &
                                    0xf0;
                                
                        
                        
                            // Even cluster: Low 12 bits being set
                            else {
                                
                                
                                    (scratch[offset + 1] & 0xf0) | new_contents
                                    & 0x0f;
                        
                        
                            DFS_WriteSector(volinfo->unit, scratch,
                                            *scratchcache, 1);
                        
                            // mirror the FAT into copy 2
                            if (DFS_OK == result)
                                
                                    DFS_WriteSector(volinfo->unit, scratch,
                                                    (*scratchcache) +
                                                    volinfo->secperfat, 1);
                
        
        
        else if (volinfo->filesystem == FAT16) {
                
                
                
                    DFS_WriteSector(volinfo->unit, scratch, *scratchcache, 1);
                
                    // mirror the FAT into copy 2
                    if (DFS_OK == result)
                        
                            DFS_WriteSector(volinfo->unit, scratch,
                                            (*scratchcache) +
                                            volinfo->secperfat, 1);
        
        
        else if (volinfo->filesystem == FAT32) {
                
                
                
                
                    (scratch[offset + 3] & 0xf0) | ((new_contents & 0x0f000000)
                                                    >> 24);
                
                    // Note well from the above: Per Microsoft's guidelines we preserve the upper
                    // 4 bits of the FAT32 cluster value. It's unclear what these bits will be used
                    // for; in every example I've encountered they are always zero.
                    result =
                    DFS_WriteSector(volinfo->unit, scratch, *scratchcache, 1);
                
                    // mirror the FAT into copy 2
                    if (DFS_OK == result)
                        
                            DFS_WriteSector(volinfo->unit, scratch,
                                            (*scratchcache) +
                                            volinfo->secperfat, 1);
        
        
        else
                
        



/*
	Convert a filename element from canonical (8.3) to directory entry (11) form
	src must point to the first non-separator character.
	dest must point to a 12-byte buffer.
*/ 
    uint8_t * DFS_CanonicalToDir(uint8_t * dest, uint8_t * src) 
{
        
        
        
        
                
                        
                        
                
                
                else if (*src == '.') {
                        
                        
                
                
                else {
                        
                
        
        



/*
	Find the first unused FAT entry
	You must provide a scratch buffer for one sector (SECTOR_SIZE) and a populated VOLINFO
	Returns a FAT32 BAD_CLUSTER value for any error, otherwise the contents of the desired
	FAT entry.
	Returns FAT32 bad_sector (0x0ffffff7) if there is no free cluster available
*/ 
    uint32_t DFS_GetFreeFAT(PVOLINFO volinfo, uint8_t * scratch) 
{
        
        
            // Search starts at cluster 2, which is the first usable cluster
            // NOTE: This search can't terminate at a bad cluster, because there might
            // legitimately be bad clusters on the disk.
            for (i = 2; i < volinfo->numclusters; i++) {
                
                
                        
                
        
        
}


/*
	Open a directory for enumeration by DFS_GetNextDirEnt
	You must supply a populated VOLINFO (see DFS_GetVolInfo)
	The empty string or a string containing only the directory separator are
	considered to be the root directory.
	Returns 0 OK, nonzero for any error.
*/ 
    uint32_t DFS_OpenDir(PVOLINFO volinfo, uint8_t * dirname,
                         PDIRINFO dirinfo) 
{
        
            // Default behavior is a regular search for existing entries
            dirinfo->flags = 0;
        
               || (strlen((char *)dirname) == 1 && dirname[0] == DIR_SEPARATOR)) {
                
                        
                        
                        
                        
                            // read first sector of directory
                            return DFS_ReadSector(volinfo->unit,
                                                  dirinfo->scratch,
                                                  volinfo->dataarea +
                                                  ((volinfo->rootdir -
                                                    2) * volinfo->secperclus),
                                                  1);
                
                
                else {
                        
                        
                        
                        
                            // read first sector of directory
                            return DFS_ReadSector(volinfo->unit,
                                                  dirinfo->scratch,
                                                  volinfo->rootdir, 1);
                
        
        
            // This is not the root directory. We need to find the start of this subdirectory.
            // We do this by devious means, using our own companion function DFS_GetNext.
            else {
                
                
                
                
                
                        
                        
                        
                        
                            // read first sector of directory
                            if (DFS_ReadSector
                                (volinfo->unit, dirinfo->scratch,
                                 volinfo->dataarea +
                                 ((volinfo->rootdir - 2) * volinfo->secperclus),
                                 1))
                                
                
                
                else {
                        
                        
                        
                        
                            // read first sector of directory
                            if (DFS_ReadSector
                                (volinfo->unit, dirinfo->scratch,
                                 volinfo->rootdir, 1))
                                
                
                
                    // skip leading path separators
                    while (*ptr == DIR_SEPARATOR && *ptr)
                        
                
                    // Scan the path from left to right, finding the start cluster of each entry
                    // Observe that this code is inelegant, but obviates the need for recursion.
                    while (*ptr) {
                        
                        
                        
                        do {
                                
                        
                        while (!result && memcmp(de.name, tmpfn, 11));
                        
                               && ((de.attr & ATTR_DIRECTORY) ==
                                   ATTR_DIRECTORY)) {
                                
                                        
                                            (uint32_t) de.startclus_l_l |
                                            
                                            | 
                                            16 | 
                                            << 24;
                                
                                
                                else {
                                        
                                            (uint32_t) de.startclus_l_l |
                                            
                                
                                
                                
                                
                                       (volinfo->unit, dirinfo->scratch,
                                        volinfo->dataarea +
                                        ((dirinfo->currentcluster -
                                          2) * volinfo->secperclus), 1))
                                        
                        
                        
                        else if (!memcmp(de.name, tmpfn, 11)
                                 && !(de.attr & ATTR_DIRECTORY))
                                
                        
                            // seek to next item in list
                            while (*ptr != DIR_SEPARATOR && *ptr)
                                
                        
                                
                
                
                        
        
        



/*
	Get next entry in opened directory structure. Copies fields into the dirent
	structure, updates dirinfo. Note that it is the _caller's_ responsibility to
	handle the '.' and '..' entries.
	A deleted file will be returned as a NULL entry (first char of filename=0)
	by this code. Filenames beginning with 0x05 will be translated to 0xE5
	automatically. Long file name entries will be returned as NULL.
	returns DFS_EOF if there are no more entries, DFS_OK if this entry is valid,
	or DFS_ERRMISC for a media error
*/ 
    uint32_t DFS_GetNext(PVOLINFO volinfo, PDIRINFO dirinfo, PDIRENT dirent) 
{
        
        
            // Do we need to read the next sector of the directory?
            if (dirinfo->currententry >= SECTOR_SIZE / sizeof(DIRENT)) {
                
                
                
                    // Root directory; special case handling 
                    // Note that currentcluster will only ever be zero if both:
                    // (a) this is the root directory, and
                    // (b) we are on a FAT12/16 volume, where the root dir can't be expanded
                    if (dirinfo->currentcluster == 0) {
                        
                            // Trying to read past end of root directory?
                            if (dirinfo->currentsector *
                                (SECTOR_SIZE / sizeof(DIRENT)) >=
                                volinfo->rootentries)
                                
                        
                            // Otherwise try to read the next sector
                            if (DFS_ReadSector
                                (volinfo->unit, dirinfo->scratch,
                                 volinfo->rootdir + dirinfo->currentsector, 1))
                                
                
                
                    // Normal handling
                    else {
                        
                                
                                
                                      && volinfo->filesystem == FAT12)
                                     || 
                                          && volinfo->filesystem == FAT16)
                                     || 
                                          && volinfo->filesystem == FAT32)) {
                                        
                                            // We are at the end of the directory chain. If this is a normal
                                            // find operation, we should indicate that there is nothing more
                                            // to see.
                                            if (!
                                                (dirinfo->flags &
                                                 DFS_DI_BLANKENT))
                                                
                                        
                                            // On the other hand, if this is a "find free entry" search,
                                            // we need to tell the caller to allocate a new cluster
                                            else
                                                
                                
                                
                                    DFS_GetFAT(volinfo, dirinfo->scratch,
                                               &tempint,
                                               dirinfo->currentcluster);
                        
                        
                             (volinfo->unit, dirinfo->scratch,
                              volinfo->dataarea +
                              ((dirinfo->currentcluster -
                                2) * volinfo->secperclus) +
                              dirinfo->currentsector, 1))
                                
                
        
        
                  &(((PDIRENT) dirinfo->scratch)[dirinfo->currententry]),
                  sizeof(DIRENT));
        
                // If this is a "find blank" then we can reuse this name.
                if (dirinfo->flags & DFS_DI_BLANKENT)
                        
                
                else
                        
        
        
                dirent->name[0] = 0;
        
        else if ((dirent->attr & ATTR_LONG_NAME) == ATTR_LONG_NAME)
                
        
        else if (dirent->name[0] == 0x05) // handle kanji filenames beginning with 0xE5
                dirent->name[0] = 0xe5;
        
        



/*
	INTERNAL
	Find a free directory entry in the directory specified by path
	This function MAY cause a disk write if it is necessary to extend the directory
	size.
	Note - di.scratch must be preinitialized to point to a sector scratch buffer
	de is a scratch structure
	Returns DFS_ERRMISC if a new entry could not be located or created
	de is updated with the same return information you would expect from DFS_GetNext
*/ 
    uint32_t DFS_GetFreeDirEnt(PVOLINFO volinfo, uint8_t * path, PDIRINFO di,
                               PDIRENT de) 
{
        
        
                
        
            // Set "search for empty" flag so DFS_GetNext knows what we're doing
            di->flags |= DFS_DI_BLANKENT;
        
            // We seek through the directory looking for an empty entry
            // Note we are reusing tempclus as a temporary result holder.
            tempclus = 0;
        
        do {
                
                
                    // Empty entry found
                    if (tempclus == DFS_OK && (!de->name[0])) {
                        
                
                
                    // End of root directory reached
                    else if (tempclus == DFS_EOF)
                        
                
                else if (tempclus == DFS_ALLOCNEW) {
                        
                        
                                
                        
                            // write out zeroed sectors to the new cluster
                            memset(di->scratch, 0, SECTOR_SIZE);
                        
                                
                                     (volinfo->unit, di->scratch,
                                      volinfo->dataarea +
                                      ((tempclus - 2) * volinfo->secperclus) +
                                      i, 1))
                                        
                        
                        
                            // Point old end cluster to newly allocated cluster
                            i = 0;
                        
                                    di->currentcluster, tempclus);
                        
                            // Update DIRINFO so caller knows where to place the new file                   
                            di->currentcluster = tempclus;
                        
                        
                        
                            // Mark newly allocated cluster as end of chain                 
                            switch (volinfo->filesystem) {
                        
                                tempclus = 0xff8;
                                break;
                        
                                tempclus = 0xfff8;
                                break;
                        
                                tempclus = 0x0ffffff8;
                                break;
                        
                                return DFS_ERRMISC;
                        
                        
                                    di->currentcluster, tempclus);
                
        
        while (!tempclus);
        
            // We shouldn't get here
            return DFS_ERRMISC;



/*
	Open a file for reading or writing. You supply populated VOLINFO, a path to the file,
	mode (DFS_READ or DFS_WRITE) and an empty fileinfo structure. You also need to
	provide a pointer to a sector-sized scratch buffer.
	Returns various DFS_* error states. If the result is DFS_OK, fileinfo can be used
	to access the file from this point on.
*/ 
    uint32_t DFS_OpenFile(PVOLINFO volinfo, uint8_t * path, uint8_t mode,
                          uint8_t * scratch, PFILEINFO fileinfo) 
{
        
        
        
        
        
        
            // larwe 2006-09-16 +1 zero out file structure
            memset(fileinfo, 0, sizeof(FILEINFO));
        
            // save access mode
            fileinfo->mode = mode;
        
            // Get a local copy of the path. If it's longer than MAX_PATH, abort.
            strncpy((char *)tmppath, (char *)path, MAX_PATH);
        
        
                
        
        
            // strip leading path separators
            while (tmppath[0] == DIR_SEPARATOR)
                
        
            // Parse filename off the end of the supplied path
            p = tmppath;
        
        
        
                p--;
        
                
        
        
                
        
                *p = 0;
        
            // At this point, if our path was MYDIR/MYDIR2/FILE.EXT, filename = "FILE    EXT" and
            // tmppath = "MYDIR/MYDIR2".
            di.scratch = scratch;
        
                
        
                
                        
                            // You can't use this function call to open a directory.
                            if (de.attr & ATTR_DIRECTORY)
                                
                        
                        
                        
                            // The reason we store this extra info about the file is so that we can
                            // speedily update the file size, modification date, etc. on a file that is
                            // opened for writing.
                            if (di.currentcluster == 0)
                                
                                    volinfo->rootdir + di.currentsector;
                        
                        else
                                
                                    volinfo->dataarea +
                                    ((di.currentcluster -
                                      2) * volinfo->secperclus) +
                                    di.currentsector;
                        
                        
                                
                                    (uint32_t) de.startclus_l_l | 
                                                                    de.
                                                                    startclus_l_h)
                                    << 8 | 
                                             startclus_h_l) << 16 | 
                                                                      de.
                                                                      startclus_h_h)
                                    << 24;
                        
                        
                        else {
                                
                                    (uint32_t) de.startclus_l_l | 
                                                                    de.
                                                                    startclus_l_h)
                                    << 8;
                        
                        
                        
                            ((uint32_t) de.filesize_1) << 8 | 
                            ((uint32_t) de.filesize_2) << 16 | 
                            ((uint32_t) de.filesize_3) << 24;
                        
                
        
        
            // At this point, we KNOW the file does not exist. If the file was opened
            // with write access, we can create it.
            if (mode & DFS_WRITE) {
                
                
                    // Locate or create a directory entry for this file
                    if (DFS_OK != DFS_GetFreeDirEnt(volinfo, tmppath, &di, &de))
                        
                
                    // put sane values in the directory entry
                    memset(&de, 0, sizeof(de));
                
                
                de.crttime_h = 0x08;
                
                
                
                
                
                
                
                
                
                    // allocate a starting cluster for the directory entry
                    cluster = DFS_GetFreeFAT(volinfo, scratch);
                
                
                
                
                
                    // update FILEINFO for our caller's sake
                    fileinfo->volinfo = volinfo;
                
                
                    // The reason we store this extra info about the file is so that we can
                    // speedily update the file size, modification date, etc. on a file that is
                    // opened for writing.
                    if (di.currentcluster == 0)
                        
                            volinfo->rootdir + di.currentsector;
                
                else
                        
                            volinfo->dataarea +
                            ((di.currentcluster - 2) * volinfo->secperclus) +
                            di.currentsector;
                
                
                
                
                
                    // write the directory entry
                    // note that we no longer have the sector containing the directory entry,
                    // tragically, so we have to re-read it
                    if (DFS_ReadSector
                        (volinfo->unit, scratch, fileinfo->dirsector, 1))
                        
                
                        sizeof(DIRENT));
                
                     (volinfo->unit, scratch, fileinfo->dirsector, 1))
                        
                
                    // Mark newly allocated cluster as end of chain                 
                    switch (volinfo->filesystem) {
                
                        cluster = 0xff8;
                        break;
                
                        cluster = 0xfff8;
                        break;
                
                        cluster = 0x0ffffff8;
                        break;
                
                        return DFS_ERRMISC;
                
                
                
                            cluster);
                
        
        



/*
	Read an open file
	You must supply a prepopulated FILEINFO as provided by DFS_OpenFile, and a
	pointer to a SECTOR_SIZE scratch buffer.
	Note that returning DFS_EOF is not an error condition. This function updates the
	successcount field with the number of bytes actually read.
*/ 
    uint32_t DFS_ReadFile(PFILEINFO fileinfo, uint8_t * scratch,
                          uint8_t * buffer, uint32_t * successcount,
                          uint32_t len) 
{
        
        
        
        
        
            // Don't try to read past EOF
            if (len > fileinfo->filelen - fileinfo->pointer)
                
        
        
        
                
                    // This is a bit complicated. The sector we want to read is addressed at a cluster
                    // granularity by the fileinfo->cluster member. The file pointer tells us how many
                    // extra sectors to add to that number.
                    sector = fileinfo->volinfo->dataarea + 
                    ((fileinfo->cluster - 2) * fileinfo->volinfo->secperclus) +
                    
                         (fileinfo->pointer,
                          fileinfo->volinfo->secperclus * SECTOR_SIZE).rem,
                         SECTOR_SIZE).quot;
                
                    // Case 1 - File pointer is not on a sector boundary
                    if (div(fileinfo->pointer, SECTOR_SIZE).rem) {
                        
                        
                            // We always have to go through scratch in this case
                            result =
                            DFS_ReadSector(fileinfo->volinfo->unit, scratch,
                                           sector, 1);
                        
                            // This is the number of bytes that we actually care about in the sector
                            // just read.
                            tempreadsize =
                            SECTOR_SIZE -
                            (div(fileinfo->pointer, SECTOR_SIZE).rem);
                        
                            // Case 1A - We want the entire remainder of the sector. After this
                            // point, all passes through the read loop will be aligned on a sector
                            // boundary, which allows us to go through the optimal path 2A below.
                            if (remain >= tempreadsize) {
                                
                                        scratch + (SECTOR_SIZE - tempreadsize),
                                        tempreadsize);
                                
                                
                                
                                
                        
                        
                            // Case 1B - This read concludes the file read operation
                            else {
                                
                                        scratch + (SECTOR_SIZE - tempreadsize),
                                        remain);
                                
                                
                                
                                
                        
                
                
                    // Case 2 - File pointer is on sector boundary
                    else {
                        
                            // Case 2A - We have at least one more full sector to read and don't have
                            // to go through the scratch buffer. You could insert optimizations here to
                            // read multiple sectors at a time, if you were thus inclined (note that
                            // the maximum multi-read you could perform is a single cluster, so it would
                            // be advantageous to have code similar to case 1A above that would round the
                            // pointer to a cluster boundary the first pass through, so all subsequent
                            // [large] read requests would be able to go a cluster at a time).
                            if (remain >= SECTOR_SIZE) {
                                
                                    DFS_ReadSector(fileinfo->volinfo->unit,
                                                   buffer, sector, 1);
                                
                                
                                
                                
                        
                        
                            // Case 2B - We are only reading a partial sector
                            else {
                                
                                    DFS_ReadSector(fileinfo->volinfo->unit,
                                                   scratch, sector, 1);
                                
                                
                                
                                
                                
                        
                
                
                
                    // check to see if we stepped over a cluster boundary
                    if (div
                        (fileinfo->pointer - bytesread,
                         fileinfo->volinfo->secperclus * SECTOR_SIZE).quot !=
                        
                             fileinfo->volinfo->secperclus * SECTOR_SIZE).quot)
                {
                        
                            // An act of minor evil - we use bytesread as a scratch integer, knowing that
                            // its value is not used after updating *successcount above
                            bytesread = 0;
                        
                              && (fileinfo->cluster >= 0xff8))
                             || 
                                  && (fileinfo->cluster >= 0xfff8))
                             || 
                                  && (fileinfo->cluster >= 0x0ffffff8)))
                                
                        
                        else
                                
                                    DFS_GetFAT(fileinfo->volinfo, scratch,
                                               &bytesread, fileinfo->cluster);
                
        
        



/*
	Seek file pointer to a given position
	This function does not return status - refer to the fileinfo->pointer value
	to see where the pointer wound up.
	Requires a SECTOR_SIZE scratch buffer
*/ 
void DFS_Seek(PFILEINFO fileinfo, uint32_t offset, uint8_t * scratch) 
{
        
        
            // larwe 9/16/06 bugfix split case 0a/0b and changed fallthrough handling
            // Case 0a - Return immediately for degenerate case
            if (offset == fileinfo->pointer) {
                
        
        
            // Case 0b - Don't allow the user to seek past the end of the file
            if (offset > fileinfo->filelen) {
                
                
                    // NOTE NO RETURN HERE!
        }
        
            // Case 1 - Simple rewind to start
            // Note _intentional_ fallthrough from Case 0b above
            if (offset == 0) {
                
                
                
        }
        
            // Case 2 - Seeking backwards. Need to reset and seek forwards
            else if (offset < fileinfo->pointer) {
                
                
                
                    // NOTE NO RETURN HERE!
        }
        
            // Case 3 - Seeking forwards
            // Note _intentional_ fallthrough from Case 2 above
            
            // Case 3a - Seek size does not cross cluster boundary - 
            // very simple case
            // larwe 9/16/06 changed .rem to .quot in both div calls, bugfix
            if (div
                (fileinfo->pointer,
                 fileinfo->volinfo->secperclus * SECTOR_SIZE).quot ==
                
                     fileinfo->volinfo->secperclus * SECTOR_SIZE).quot) {
                
        
        
            // Case 3b - Seeking across cluster boundary(ies)
            else {
                
                    // round file pointer down to cluster boundary
                    fileinfo->pointer =
                    div(fileinfo->pointer,
                        fileinfo->volinfo->secperclus * SECTOR_SIZE).quot *
                    
                
                    // seek by clusters
                    // larwe 9/30/06 bugfix changed .rem to .quot in both div calls
                    while (div
                           (fileinfo->pointer,
                            fileinfo->volinfo->secperclus * SECTOR_SIZE).quot !=
                           
                                fileinfo->volinfo->secperclus *
                                SECTOR_SIZE).quot) {
                        
                            DFS_GetFAT(fileinfo->volinfo, scratch, &tempint,
                                       fileinfo->cluster);
                        
                            // Abort if there was an error
                            if (fileinfo->cluster == 0x0ffffff7) {
                                
                                
                                
                        
                        
                            SECTOR_SIZE * fileinfo->volinfo->secperclus;
                
                
                    // since we know the cluster is right, we have no more work to do
                    fileinfo->pointer = offset;
        



/*
	Delete a file
	scratch must point to a sector-sized buffer
*/ 
    uint32_t DFS_UnlinkFile(PVOLINFO volinfo, uint8_t * path,
                            uint8_t * scratch) 
{
        
        
        
        
        
            // DFS_OpenFile gives us all the information we need to delete it
            if (DFS_OK != DFS_OpenFile(volinfo, path, DFS_READ, scratch, &fi))
                
        
            // First, read the directory sector and delete that entry
            if (DFS_ReadSector(volinfo->unit, scratch, fi.dirsector, 1))
                
        
        
                
        
            // Now follow the cluster chain to free the file space
            while (!((volinfo->filesystem == FAT12 && fi.firstcluster >= 0x0ff7)
                     || 
                          && fi.firstcluster >= 0xfff7)
                     || 
                          && fi.firstcluster >= 0x0ffffff7))) {
                
                
                    DFS_GetFAT(volinfo, scratch, &cache, fi.firstcluster);
                
        
        



/*
	Write an open file
	You must supply a prepopulated FILEINFO as provided by DFS_OpenFile, and a
	pointer to a SECTOR_SIZE scratch buffer.
	This function updates the successcount field with the number of bytes actually written.
*/ 
    uint32_t DFS_WriteFile(PFILEINFO fileinfo, uint8_t * scratch,
                           uint8_t * buffer, uint32_t * successcount,
                           uint32_t len) 
{
        
        
        
        
        
            // Don't allow writes to a file that's open as readonly
            if (!(fileinfo->mode & DFS_WRITE))
                
        
        
        
                
                    // This is a bit complicated. The sector we want to read is addressed at a cluster
                    // granularity by the fileinfo->cluster member. The file pointer tells us how many
                    // extra sectors to add to that number.
                    sector = fileinfo->volinfo->dataarea + 
                    ((fileinfo->cluster - 2) * fileinfo->volinfo->secperclus) +
                    
                         (fileinfo->pointer,
                          fileinfo->volinfo->secperclus * SECTOR_SIZE).rem,
                         SECTOR_SIZE).quot;
                
                    // Case 1 - File pointer is not on a sector boundary
                    if (div(fileinfo->pointer, SECTOR_SIZE).rem) {
                        
                        
                            // We always have to go through scratch in this case
                            result =
                            DFS_ReadSector(fileinfo->volinfo->unit, scratch,
                                           sector, 1);
                        
                            // This is the number of bytes that we don't want to molest in the
                            // scratch sector just read.
                            tempsize = div(fileinfo->pointer, SECTOR_SIZE).rem;
                        
                            // Case 1A - We are writing the entire remainder of the sector. After
                            // this point, all passes through the read loop will be aligned on a
                            // sector boundary, which allows us to go through the optimal path
                            // 2A below.
                            if (remain >= SECTOR_SIZE - tempsize) {
                                
                                        SECTOR_SIZE - tempsize);
                                
                                        
                                            DFS_WriteSector(fileinfo->volinfo->
                                                            unit, scratch,
                                                            sector, 1);
                                
                                
                                
                                
                                        
                                
                                
                        
                        
                            // Case 1B - This concludes the file write operation
                            else {
                                
                                
                                        
                                            DFS_WriteSector(fileinfo->volinfo->
                                                            unit, scratch,
                                                            sector, 1);
                                
                                
                                
                                        
                                
                                
                                
                        
                
                // Case 2 - File pointer is on sector boundary
                else {
                        
                            // Case 2A - We have at least one more full sector to write and don't have
                            // to go through the scratch buffer. You could insert optimizations here to
                            // write multiple sectors at a time, if you were thus inclined. Refer to
                            // similar notes in DFS_ReadFile.
                            if (remain >= SECTOR_SIZE) {
                                
                                    DFS_WriteSector(fileinfo->volinfo->unit,
                                                    buffer, sector, 1);
                                
                                
                                
                                
                                        
                                
                                
                        
                        
                            // Case 2B - We are only writing a partial sector and potentially need to
                            // go through the scratch buffer.
                            else {
                                
                                    // If the current file pointer is not yet at or beyond the file
                                    // length, we are writing somewhere in the middle of the file and
                                    // need to load the original sector to do a read-modify-write.
                                    if (fileinfo->pointer < fileinfo->filelen) {
                                        
                                            DFS_ReadSector(fileinfo->volinfo->
                                                           unit, scratch,
                                                           sector, 1);
                                        
                                                
                                                        remain);
                                                
                                                    DFS_WriteSector
                                                    (fileinfo->volinfo->unit,
                                                     scratch, sector, 1);
                                        
                                
                                
                                else {
                                        
                                            DFS_WriteSector(fileinfo->volinfo->
                                                            unit, buffer,
                                                            sector, 1);
                                
                                
                                
                                
                                        
                                
                                
                                
                        
                
                
                
                    // check to see if we stepped over a cluster boundary
                    if (div
                        (fileinfo->pointer - byteswritten,
                         fileinfo->volinfo->secperclus * SECTOR_SIZE).quot !=
                        
                             fileinfo->volinfo->secperclus * SECTOR_SIZE).quot)
                {
                        
                        
                            // We've transgressed into another cluster. If we were already at EOF,
                            // we need to allocate a new cluster.
                            // An act of minor evil - we use byteswritten as a scratch integer, knowing
                            // that its value is not used after updating *successcount above
                            byteswritten = 0;
                        
                        
                            DFS_GetFAT(fileinfo->volinfo, scratch,
                                       &byteswritten, fileinfo->cluster);
                        
                            // Allocate a new cluster?
                            if (((fileinfo->volinfo->filesystem == FAT12)
                                 && (fileinfo->cluster >= 0xff8))
                                || 
                                     && (fileinfo->cluster >= 0xfff8))
                                || 
                                     && (fileinfo->cluster >= 0x0ffffff8))) {
                                
                                
                                    DFS_GetFreeFAT(fileinfo->volinfo, scratch);
                                
                                if (tempclus == 0x0ffffff7)
                                        
                                
                                    // Link new cluster onto file
                                    DFS_SetFAT(fileinfo->volinfo, scratch,
                                               &byteswritten, lastcluster,
                                               tempclus);
                                
                                
                                    // Mark newly allocated cluster as end of chain                 
                                    switch (fileinfo->volinfo->filesystem) {
                                
                                        tempclus = 0xff8;
                                        break;
                                
                                        tempclus = 0xfff8;
                                        break;
                                
                                        tempclus = 0x0ffffff8;
                                        break;
                                
                                        return DFS_ERRMISC;
                                
                                
                                            &byteswritten, fileinfo->cluster,
                                            tempclus);
                                
                        
                        
                            // No else clause is required.
                }
        
        
            // Update directory entry
            if (DFS_ReadSector
                (fileinfo->volinfo->unit, scratch, fileinfo->dirsector, 1))
                
        
            fileinfo->filelen & 0xff;
        
            (fileinfo->filelen & 0xff00) >> 8;
        
            (fileinfo->filelen & 0xff0000) >> 16;
        
            (fileinfo->filelen & 0xff000000) >> 24;
        
             (fileinfo->volinfo->unit, scratch, fileinfo->dirsector, 1))
                
        


