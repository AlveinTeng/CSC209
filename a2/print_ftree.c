#include <stdio.h>
#include <stdlib.h>

#include "ftree.h"


int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage:\n\tprint_ftree PATH\n");
        return 1;
    }

    struct TreeNode *root = generate_ftree(argv[1]);
    if (root == NULL) {
        return 1;
    }

    print_ftree(root);

    deallocate_ftree(root); 

    return 0;
}

