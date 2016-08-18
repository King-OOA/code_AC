#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "make_patset.h"
#include "make_data.h"
#include "textools.h"
#include "AC.h"

/* void make_rand_pats(void); */
/* void make_random_text(void); */

int main(int argc, char *argv[])
{
    FILE *fp_text;
    patset_t *pat_set;
    char *text_file_name, *pats_file_name;
    
   /* fill_file("/home/pz/text", 100, 128); */
   /*   gen_patterns("/home/pz/text", 300000, 10, 20, "/home/pz/pat", "\n\b\t"); */

    if (argc == 3) {
        text_file_name = argv[1];
        pats_file_name = argv[2];
    } else {
        text_file_name = TEXT_FILE;
        pats_file_name = PATS_FILE;
    }
    
    printf("text file: %s\n", text_file_name);
    printf("pats file: %s\n", pats_file_name);
    
    if (!(fp_text = fopen(text_file_name, "r"))) {
        perror(text_file_name);
        exit(EXIT_FAILURE);
    }
    
    printf("reading pats....  ");
    fflush(stdout);
    pat_set = cre_pat_set(pats_file_name);
    printf("Done!\n");

    AC(fp_text, pat_set);
    
    des_pat_set(pat_set);
    fclose(fp_text);
    
    return 0;
}



