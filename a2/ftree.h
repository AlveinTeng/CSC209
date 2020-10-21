#ifndef _FTREE_H_
#define _FTREE_H_

/*
 * Data structure for storing information about a single file.
 * For directories, contents is the linked list of files in the directory.
 * For regular files and links, contents is NULL.
 * next is the next file in the directory (or NULL).
 */
struct TreeNode {
    char *fname;
    int permissions;
    char type;                   // 'd' (directory), '-' (regular file), or 'l' (link)

    struct TreeNode *contents;   // For directories

    struct TreeNode *next;       
};


/*
 * An FTree is a dynamically allocated tree structure that contains
 * information about the files in a file system. An FTree is represented by
 * a single TreeNode which is the root of the tree.
 */

// Function for generating an FTree given a root filename.
struct TreeNode *generate_ftree(const char *fname);

// Function for printing the TreeNodes encountered on a preorder traversal of an FTree.
void print_ftree(struct TreeNode *root);

// Function for deallocating all dynamically allocated memory of an FTree.
void deallocate_ftree(struct TreeNode *root);

#endif // _FTREE_H_
