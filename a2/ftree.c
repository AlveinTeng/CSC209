#include <stdio.h>
// Add your system includes here.
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "ftree.h"
struct TreeNode *generate_helper(const char *fname, char *path){
	char new_path[strlen(fname) + strlen(path) + 2];
	if (strcmp(path, "") == 0){
	strcpy(new_path, fname);
	}else{
	strcpy(new_path, path);
	strcat(new_path, "/");
	strcat(new_path, fname);
	}
	struct stat stat_buf;
	if (lstat(new_path, &stat_buf) == -1){
	fprintf(stderr, "The path (%s) does not point to an existing entry!\n", new_path);
        return NULL;
	}
	struct TreeNode *treenode = malloc(sizeof(struct TreeNode));
	treenode->fname = malloc(sizeof(char) * strlen(fname + 1));
	treenode->permissions = stat_buf.st_mode & 0777;
	strcpy(treenode->fname, fname);

	if(S_ISDIR(stat_buf.st_mode)){
	treenode->type = 'd';
	DIR *d_ptr = opendir(new_path);
	if(d_ptr == NULL){
		fprintf(stderr, "The path (%s) does not point to an existing entry!\n", new_path);
        return NULL;
	}
	struct dirent *entry_ptr = readdir(d_ptr);

	while(entry_ptr !=NULL  && entry_ptr->d_name[0] == '.'){
		entry_ptr = readdir(d_ptr);
	}
	if(entry_ptr != NULL){
		treenode ->contents = generate_helper(entry_ptr->d_name , new_path);
	}else{
		treenode->contents = NULL;
	}
	closedir(d_ptr);
	}else {
		treenode->contents = NULL;
		if(S_ISREG(stat_buf.st_mode)){
		treenode -> type = '-';
	}else if(S_ISLNK(stat_buf.st_mode)){
		treenode ->type = 'l';
	}
	}

	if (strcmp(path, "") == 0) {
		treenode->next = NULL;
	} else {
	DIR *cur_ptr = opendir(path);
		if(cur_ptr == NULL){
		fprintf(stderr, "The path (%s) does not point to an existing entry!\n", path);
        	return NULL;
		}
		struct dirent *cur_entry_ptr;
		cur_entry_ptr = readdir(cur_ptr);
		while (strcmp(cur_entry_ptr->d_name, fname) != 0) {
            cur_entry_ptr = readdir(cur_ptr);
        }
        cur_entry_ptr = readdir(cur_ptr);
		while(cur_entry_ptr !=NULL  && cur_entry_ptr->d_name[0] == '.'){
			cur_entry_ptr = readdir(cur_ptr);
		}
		if(cur_entry_ptr != NULL){
			treenode ->next = generate_helper(cur_entry_ptr-> d_name, path);
		}else{
			treenode->next = NULL;
		}
		closedir(cur_ptr);
	}

return treenode;






}

/*
 * Returns the FTree rooted at the path fname.
 *
 * Use the following if the file fname doesn't exist and return NULL:
 * fprintf(stderr, "The path (%s) does not point to an existing entry!\n", fname);
 *
 */
struct TreeNode *generate_ftree(const char *fname) {

    // Your implementation here.
	struct TreeNode *treenode  = generate_helper(fname, "");
    // Hint: consider implementing a recursive helper function that
    // takes fname and a path.  For the initial call on the 
    // helper function, the path would be "", since fname is the root
    // of the FTree.  For files at other depths, the path would be the
    // file path from the root to that file.

    return treenode;
}


/*
 * Prints the TreeNodes encountered on a preorder traversal of an FTree.
 *
 * The only print statements that you may use in this function are:
 * printf("===== %s (%c%o) =====\n", root->fname, root->type, root->permissions)
 * printf("%s (%c%o)\n", root->fname, root->type, root->permissions)
 *
 */
void print_ftree(struct TreeNode *root) {
	
    // Here's a trick for remembering what depth (in the tree) you're at
    // and printing 2 * that many spaces at the beginning of the line.
    static int depth = 0;
    printf("%*s", depth * 2, "");
    // Your implementation here.

	if (root == NULL) {
        return;
    }
    if (root->type == 'd') {
        printf("===== %s (%c%o) =====\n", root->fname, root->type, root->permissions);
        depth += 1;
        if (root->contents != NULL) {
            print_ftree(root->contents);
        }
        depth -= 1;
    } else {
        printf("%s (%c%o)\n", root->fname, root->type, root->permissions);
    }
    if (root->next != NULL) {
        print_ftree(root->next);
    }
}


/* 
 * Deallocate all dynamically-allocated memory in the FTree rooted at node.
 * 
 */
void deallocate_ftree (struct TreeNode *node) {
   // Your implementation here.
   if (node == NULL) {
        return;
    }
    free(node->fname);
    if (node->contents != NULL) {
        deallocate_ftree(node->contents);
    }
    if(node->next != NULL) {
        deallocate_ftree(node->next);
    }
    free(node);
}
