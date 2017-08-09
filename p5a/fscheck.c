#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <stddef.h>
#include "fsinclude.h"

//declarations of helper functions
int compare_bitmap(char *b1, char *b2, int size);
uint get_block_addr(struct dinode *currd, int indirect, uint offset);
void depth_first(int inum, int parent, char* newbm, int *inode_ref);

void *image;  // ptr to the file image
struct superblock *superb;  //superblock ptr

void *db; //data bitmap
struct dinode *di_ptr;  //dinode ptr
char *bm;  // bitmap pointer


// dfs to search thru entire fs
void depth_first(int inum, int parent, char* newbm, int *ref) {
  
	struct dinode *curr_di_ptr = di_ptr + inum;

  // no . or ..
  if (curr_di_ptr->size == 0 && curr_di_ptr->type == T_DIR) {
    fprintf(stderr, "ERROR: directory not properly formatted.\n");
		fflush(stderr);
  }

	// inode referred to in directory but markeed free
	if (curr_di_ptr->type == 0) {
    fprintf(stderr, "ERROR: inode referred to in directory but marked free.\n");
		fflush(stderr);
    exit(1);
  }

  //changes ref_count of inode
  ref[inum]++;
  if (ref[inum] > 1 && curr_di_ptr->type == T_DIR) {
    fprintf(stderr, "ERROR: directory appears more than once in file system.\n");
		fflush(stderr);
    exit(1);
  }

	//change to 1 if it is indirect block
  int flag = 0;
  int offset;
  for (offset = 0; offset < curr_di_ptr->size; offset += BSIZE) {
    uint address = get_block_addr(curr_di_ptr, flag, offset);
    //address_checker(addr);

///start new (uncomment function)/////
	  if (address == 0) {
	    continue;
  	}	
		  // OoB address
  	if (address >= (superb->nblocks + 4 + superb->ninodes/IPB + superb->nblocks/BPB)) {
    	fprintf(stderr, "ERROR: bad address in inode.\n");
			fflush(stderr);
    	exit(1);
  	}
		if (address < (superb->ninodes/IPB + superb->nblocks/BPB + 4)) {
			fprintf(stderr, "ERROR: bad address in inode.\n");
      fflush(stderr);
      exit(1);
		}

		//marked free but being used
  	char b = *((bm + (address/8)));
  	if (!((b>>(address%8)) & 0x01)) {
    	fprintf(stderr, "ERROR: address used by inode but marked free in bitmap.\n");
			fflush(stderr);

    	exit(1);
 	  }
////end new////

    if (flag == 0 && offset / BSIZE == NDIRECT) {
      offset -= BSIZE;
      flag = 1;

    }

		// checking for duplicate
    if (ref[inum] == 1) {
      char b = *((newbm+ (address / 8)));
      if ((b>>(address%8)) & 0x01) {
        fprintf(stderr, "ERROR: address used more than once.\n");
				fflush(stderr);
        exit(1);
      } 
			else {
        b = b | (0x01<<(address%8));
        *((newbm + (address/8))) = b;
      }
    }

    if (curr_di_ptr->type == T_DIR) {
      struct dirent *dirent_ptr = (struct dirent *) (image + address * BSIZE);

      //looks for . or ..
      if (offset == 0) {
        if (strcmp(dirent_ptr->name, ".") != 0) {
          fprintf(stderr, "ERROR: directory not properly formatted.\n");
					fflush(stderr);
          exit(1);
        }
        if (strcmp((dirent_ptr + 1)->name, "..") != 0) {
          fprintf(stderr, "ERROR: directory not properly formatted.\n");
					fflush(stderr);
          exit(1);
        }


        //checks for parent
        if ((dirent_ptr + 1)->inum != parent) {
          if (inum != ROOTINO) {
						fprintf(stderr, "ERROR: parent directory mismatch.\n");
						fflush(stderr);
          } 
					else {
						fprintf(stderr, "ERROR: root directory does not exist.\n");
						fflush(stderr);
          }
          exit(1);
        }

        dirent_ptr = dirent_ptr+ 2;
      }
		//}

			void* dfscheck = (((address+1) * BSIZE) + image);
      for (dirent_ptr; dirent_ptr < (struct dirent*)dfscheck; dirent_ptr++) {
        if (dirent_ptr->inum == 0) {
          continue;
        }
        //recursively search thru fs
        depth_first(dirent_ptr->inum, inum, newbm, ref);
      }

    }
  //}}}
}


}




// if bitmaps aren't same -> return 1, otherwise return 0
int compare_bitmap(char *b1, char *b2, int size) {
  int count;

  for (count= 0; count < size; ++count)
    if (*(b1++) != *(b2++))
      return 1;

  return 0;
}


//get the block number
uint get_block_addr(struct dinode *currd, int indirect, uint offset) {
  if (offset / BSIZE <= NDIRECT && !indirect) {
    return currd->addrs[offset / BSIZE];
  } else {
    return *((uint*) (image + currd->addrs[NDIRECT] * BSIZE) + offset / BSIZE - NDIRECT);
  }

}






// main fn
int main(int argc, char** argv)
{
  int file = open(argv[1], O_RDONLY);
	//opening fails
  if (file == -1) {
    fprintf(stderr, "image not found.\n");
		fflush(stderr);
    exit(1);
  }

  int ret_code;
  struct stat stat_str;

  ret_code = fstat(file, &stat_str);
  assert(ret_code == 0);

  
  image = mmap(NULL, stat_str.st_size, PROT_READ, MAP_PRIVATE, file, 0);
  assert(image != MAP_FAILED);

  superb = (struct superblock *) (image + BSIZE);

 
 	db = (void *) (image + (superb->ninodes/IPB + superb->nblocks/BPB + 4) * BSIZE);
  di_ptr = (struct dinode *) (image + 2 * BSIZE); //dinode ptr
  bm = (char *) (image + (superb->ninodes / IPB + 3) * BSIZE); //bitmap ptr

	//creating inode references, data offset #, and bitmap size
	int inode_ref[superb->ninodes + 1];
  int d_offset = superb->ninodes/IPB + superb->nblocks/BPB + 4;
	int bm_sz = ((superb->ninodes/IPB + superb->nblocks/BPB + superb->nblocks + 4) / 8);
	
	//initializing
  memset(inode_ref, 0, (superb->ninodes + 1) * sizeof(int));
  
	//new bitmap size of old bitmap
	char new_bm[bm_sz];

  // set up the new bitmap
  memset(new_bm, 0, bm_sz);
  memset(new_bm, 0xFF, d_offset / 8);
  char last = 0x00;


	int count;
  for (count = 0; count< d_offset % 8; ++count) {
    last = (last << 1) | 0x01;
  }
  new_bm[d_offset / 8] = last;

  // examine root dir
  if (!(di_ptr + ROOTINO) || (di_ptr + ROOTINO)->type != T_DIR) {
    fprintf(stderr, "ERROR: root directory does not exist.\n");
		fflush(stderr);
    exit(1);
  }

 	//dfs
  depth_first(ROOTINO, ROOTINO, new_bm, inode_ref);


  
	//dinode ptr to use for error checking
	struct dinode *curr_di_ptr = di_ptr;

///	ERROR CHECKING ///
  for(count = 1; count<superb->ninodes; count++) {
    curr_di_ptr++;
    // //Unallocated
    if (curr_di_ptr->type == 0) {
      continue;
    }
	
    //ref_count wrong
    if (inode_ref[count] != curr_di_ptr->nlink) {
      fprintf(stderr, "ERROR: bad reference count for file.\n");
			fflush(stderr);
      exit(1);
    }

	  //free but marked as if its used
  	if (compare_bitmap(bm, new_bm, bm_sz)) {
   	  fprintf(stderr, "ERROR: bitmap marks block in use but it is not in use.\n");
			fflush(stderr);
    	exit(1);
 	  }


    //inode marked for use but can't be found in directory
    if (inode_ref[count] == 0) {
      fprintf(stderr, "ERROR: inode marked use but not found in a directory.\n");
			fflush(stderr);
      exit(1);
    }

    //link count wrong
    if (curr_di_ptr->nlink > 1 && curr_di_ptr->type == T_DIR ) {
      fprintf(stderr, "ERROR: directory appears more than once in file system.\n");
			fflush(stderr);
      exit(1);
    }

    //type not valid
    if (curr_di_ptr->type != T_DIR && curr_di_ptr->type != T_FILE && curr_di_ptr->type != T_DEV) {

      fprintf(stderr, "ERROR: bad inode.\n");
			fflush(stderr);
      exit(1);
    }


  }

	//}
  return 0;
}

//}
