#ifndef FSINCLUDE_H_   /* Include guard */
#define FSINCLUDE_H_


#define T_DIR 1
#define T_FILE 2
#define T_DEV 3

#define ROOTINO 1  // root i-number
#define BSIZE 512  // block size

#define NDIRECT 12
#define NINDIRECT (BSIZE / sizeof(uint))
#define MAXFILE (NDIRECT + NINDIRECT)

// Inodes per block.
#define IPB           (BSIZE / sizeof(struct dinode))

// Bits per block
#define BPB           (BSIZE*8)

// On-disk inode structure
struct dinode {
  short type;           // File type
  short major;          // Major device number (T_DEV only)
  short minor;          // Minor device number (T_DEV only)
  short nlink;          // Number of links to inode in file system
  uint size;            // Size of file (bytes)
  uint addrs[NDIRECT+1];   // Data block addresses
};

// Directory is a file containing a sequence of dirent structures.
#define DIRSIZ 14

struct dirent {
  ushort inum;
  char name[DIRSIZ];
};

// File system super block
struct superblock {
  uint size;         // Size of file system image (blocks)
  uint nblocks;      // Number of data blocks
  uint ninodes;      // Number of inodes.
};

#endif // FSINCLUDE_H_
