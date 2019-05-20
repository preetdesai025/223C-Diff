//
//  diff_02.c
//  diff
//
//  Created by William McCarthy on 4/29/19.
//  Copyright © 2019 William McCarthy. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#include "diff.h"
#include "para.c"
#include "util.c"


void version(void) {
  printf("\n\n\ndiff (CSUF diffutils) 1.0.0\n");
  printf("Copyright (C) 2014 CSUF\n");
  printf("This program comes with NO WARRANTY, to the extent permitted by law.\n");
  printf("You may redistribute copies of this program\n");
  printf("under the terms of the GNU General Public License.\n");
  printf("For more information about these matters, see the file named COPYING.\n");
  printf("Written by William McCarthy, Tony Stark, and Dr. Steven Strange\n");
}

void todo_list(void) {
  printf("\n\n\nTODO: check line by line in a paragraph, using '|' for differences");
  printf("\nTODO: this starter code does not yet handle printing all of fin1's paragraphs.");
  printf("\nTODO: handle the rest of diff's options\n");
}

char buf[BUFLEN];
char *strings1[MAXSTRINGS], *strings2[MAXSTRINGS];
int showversion = 0, showbrief = 0, ignorecase = 0, report_identical = 0, showsidebyside = 0;
int showleftcolumn = 0, showunified = 0, showcontext = 0, suppresscommon = 0, diffnormal = 0;

int count1 = 0, count2 = 0;


void loadfiles(const char* filename1, const char* filename2) {
  memset(buf, 0, sizeof(buf));
  memset(strings1, 0, sizeof(strings1));
  memset(strings2, 0, sizeof(strings2));
  
  FILE *fin1 = openfile(filename1, "r");
  FILE *fin2 = openfile(filename2, "r");
  
  while (!feof(fin1) && fgets(buf, BUFLEN, fin1) != NULL) { strings1[count1++] = strdup(buf); }  
  while (!feof(fin2) && fgets(buf, BUFLEN, fin2) != NULL) { strings2[count2++] = strdup(buf); }  fclose(fin2); fclose(fin1);
}

void print_option(const char* name, int value) { printf("%17s: %s\n", name, yesorno(value)); }

void diff_output_conflict_error(void) {
  fprintf(stderr, "diff: conflicting output style options\n");
  fprintf(stderr, "diff: Try `diff --help' for more information.)\n");
  exit(CONFLICTING_OUTPUT_OPTIONS);
}

void setoption(const char* arg, const char* s, const char* t, int* value) {
  if ((strcmp(arg, s) == 0) || ((t != NULL && strcmp(arg, t) == 0))) {
    *value = 1;
  }
}

void showoptions(const char* file1, const char* file2) {
  printf("diff options...\n");
  print_option("diffnormal", diffnormal);
  print_option("show_version", showversion);
  print_option("show_brief", showbrief);
  print_option("ignorecase", ignorecase);
  print_option("report_identical", report_identical);
  print_option("show_sidebyside", showsidebyside);
  print_option("show_leftcolumn", showleftcolumn);
  print_option("suppresscommon", suppresscommon);
  print_option("showcontext", showcontext);
  print_option("show_unified", showunified);
  
  printf("file1: %s,  file2: %s\n\n\n", file1, file2);
  
  printline();
}

int file_contents_identical(void) {
  para* p = para_first(strings1, count1);
  para* q = para_first(strings2, count2);
  while (p != NULL && q != NULL && para_equal(p, q)) {
    p = para_next(p);
    q = para_next(q);
  }
  return p == NULL && q == NULL;
}

int check_identical(const char* filename1, const char* filename2) {
  return strcmp(filename1, filename2) == 0 || file_contents_identical() == 1;
}

void init_options_files(int argc, const char* argv[]) {
  int cnt = 0;
  const char* files[2] = { NULL, NULL };
  
  while (argc-- > 0) {
    const char* arg = *argv;
    setoption(arg, "-v",       "--version",                  &showversion);
    setoption(arg, "-q",       "--brief",                    &showbrief);
    setoption(arg, "-i",       "--ignore-case",              &ignorecase);
    setoption(arg, "-s",       "--report-identical-files",   &report_identical);
    setoption(arg, "--normal", NULL,                         &diffnormal);
    setoption(arg, "-y",       "--side-by-side",             &showsidebyside);
    setoption(arg, "--left-column", NULL,                    &showleftcolumn);
    setoption(arg, "--suppress-common-lines", NULL,          &suppresscommon);
    setoption(arg, "-c",       "--context",                  &showcontext);
    setoption(arg, "-u",       "showunified",                &showunified);
    if (arg[0] != '-') {
      if (cnt == 2) {
        fprintf(stderr, "apologies, this version of diff only handles two files\n");
        fprintf(stderr, "Usage: ./diff [options] file1 file2\n");
        exit(TOOMANYFILES_ERROR);
      } else { files[cnt++] = arg; }
    }
    ++argv;   // DEBUG only;  move increment up to top of switch at release
  }

  if (!showcontext && !showunified && !showsidebyside && !showleftcolumn) {
    diffnormal = 1;
  }
  
  if (showversion) { version();  exit(0); }
  
  if (((showsidebyside || showleftcolumn) && (diffnormal || showcontext || showunified)) ||
      (showcontext && showunified) || (diffnormal && (showcontext || showunified))) {

    diff_output_conflict_error();
  }
  
//  showoptions(files[0], files[1]);
  loadfiles(files[0], files[1]);

 int identical = check_identical(files[0], files[1]);

 if (identical || report_identical || showbrief) {
    if (report_identical && identical) {
      printf("files: %s and %s are identical\n", files[0], files[1]);
      exit(0);
    } else if (showbrief && !identical) {
      printf("files: %s and %s are different\n", files[0], files[1]);
      exit(0);
    } else { diffnormal = 1; }
  }

 if (showsidebyside || suppresscommon) {
  para* p = para_first(strings1, count1);
  para* q = para_first(strings2, count2);
  para* qlast = q;
   
  int foundmatch = 0, i = 0;
  
  if(showsidebyside && suppresscommon){
	while (p != NULL) {
    qlast = q;
    foundmatch = 0;
    while (q != NULL && (foundmatch = para_equal(p, q)) == 0) {
      q = para_next(q);
    }
    q = qlast;

    if (foundmatch) {
      while ((foundmatch = para_equal(p, q)) == 0 && (i = linechecker(p,q)) != 1) {
        para_print(q, printright);
        q = para_next(q);
        qlast = q;
      }
      p = para_next(p);
      q = para_next(q);
  } else {
      para_print(p, printleft);
      p = para_next(p);
    } // else
  } // while p != NULL
    while (q != NULL) {
    para_print(q, printright);
    q = para_next(q);
   } // end while

}else{
  
  while (p != NULL) {
    qlast = q;
    foundmatch = 0;
    while (q != NULL && (foundmatch = para_equal(p, q)) == 0) { q = para_next(q); }
    q = qlast;

    if (foundmatch) {
      while ((foundmatch = para_equal(p, q)) == 0 && (i = linechecker(p,q)) != 1) {
        para_print(q, printright);
        q = para_next(q);
        qlast = q;
      }
      if ((foundmatch = para_equal(p, q)) == 1 && (i = linechecker(p, q)) != 1 ) {
	 paraprint(p, q, i, print_both);
	}
      else { para_print(q, printboth); }
      p = para_next(p);
      q = para_next(q);
    } else {
      para_print(p, printleft);
      p = para_next(p);
    } // else
  } // while p != NULL
  while (q != NULL) {
    para_print(q, printright);
    q = para_next(q);
  } // end while
 }//end else
   exit(0);
 }// end sbs func. */


}// end init func.


int main(int argc, const char * argv[]) {
  init_options_files(--argc, ++argv);

//  para_printfile(strings1, count1, printleft);
//  para_printfile(strings2, count2, printright);
  
  para* p = para_first(strings1, count1);
  para* q = para_first(strings2, count2);
  int foundmatch = 0; int end, i = 0;

  para* qlast = q;
  while (p != NULL) {
    qlast = q;
    foundmatch = 0;
    while (q != NULL && (foundmatch = para_equal(p, q)) == 0) {
	//char* s = para_info(q);
	//printf("%s" , s);
      q = para_next(q);
    }
    q = qlast;

    if (foundmatch) {
      while ((foundmatch = para_equal(p, q)) == 0 && (i = linechecker(p, q)) != 1) {
	printf("%da%d,%d\n", q->start, q->start+1, q->stop+1);
        para_print(q, print_right);
        q = para_next(q);
        qlast = q;
      }
      while ((foundmatch = para_equal(p,q)) == 1 && (i = linechecker(p, q)) != 1) {
        printf("%dc%d\n", q->start, q->start+1);
	print_left(p->base[i]); printline(); print_right(q->base[i+1]); break;
      }
      p = para_next(p);
      q = para_next(q);
    } else {
      printf("%d,%dd%d\n", p->start, p->stop, p->stop+1);
      para_print(p, print_left);
      p = para_next(p);
      end = p->stop;
    }
  }
  while (q != NULL) {
    printf("%da%d,%d\n", end, q->start, q->stop+1);
    para_print(q, print_right);
    q = para_next(q);
  }
  
  return 0;
} // main
