#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#define URLS_FNAME "URLs.txt"
#define REPETITIONS 2500
#define MAX_URL_SIZE 1028
#define MAX_FNAME 64
#define MAX_LNAME 64
#define MAX_AGE_LEN 4
#define MAX_AGE 128
#define CSV_HEADER "fname, lname, age"


typedef struct Tfullname {
    char *fname;
    char *lname;
} Tfullname;


//globals
unsigned int max_csv_line = MAX_FNAME + MAX_LNAME + MAX_AGE_LEN + 2;
unsigned int Age_counter[MAX_AGE];
Tfullname Age_dict[MAX_AGE];

//functions
int my_stoi(unsigned int *n, char *s) {
    unsigned int x, i;
    x = i = 0;

    while (s[i] != 0) {
        if (s[i] > 47 && s[i] < 58) {
            x = x*10 + s[i] - 48;
        }
        else {
            return 1;
        }
        i++;
    }
    *n = x;
    return 0;
}

//returns 0 if everything worked
//return 1 if it reached the end of file and read no data
//returns 2 if string was too long (it then continues reading but doesn't add to the buffer)
int readline(char *s, unsigned int size, FILE *stream) {
    char c;
    unsigned int i;
    for (i = 0; i < size - 1; i++) {
        c = (char)fgetc(stream);
        if (c == '\n' || c == EOF) {
            break;
        }
        *s = c;
        s++;
    }
    *s = 0;

    //check for various errors
    //We found an end of file without reading any data
    if (i == 0 && c == EOF) {
        return 1;
    }

    //we never found \n or EOF, so too long
    //Here, keep going until we reach the end of the file or a newline but don't add anything else to the buffer
    if (i == size - 1) {
        while (c != '\n' && c != EOF) {
            c = (char)fgetc(stream);
        }
        return 2;
    }

    //everything worked fine
    return 0;
}

int unpack_row(char *fname, char *lname, char *age_arr, char *csv_line) {
    char c;
    unsigned int i, j;

    //Fill in the firstname
    for (i = 0; i < MAX_FNAME - 1; i++) {
        c = csv_line[i];
        if (c == ',' || c == 0) {
            break;
        }
        fname[i] = c;
    }
    fname[i] = 0;
    if (c == 0 || i == MAX_FNAME - 1) {
        return 1;
    }
    i += 2;

    //fill in the last name
    for (j = 0; j < MAX_LNAME - 1; j++) {
        c = csv_line[i];
        if (c == ',' || c == 0) {
            break;
        }
        lname[j] = c;
        i++;
    }
    lname[j] = 0;
    if (c == 0 || j == MAX_LNAME - 1) {
        return 1;
    }
    i += 2;

    //fill in the age
    for (j = 0; j < MAX_AGE_LEN - 1; j++) {
        c = csv_line[i];
        if (c == ',' || c == 0) {
            break;
        }
        age_arr[j] = c;
        i++;
    }
    age_arr[j] = 0;
    if (c == ',' || j == MAX_AGE_LEN - 1) {
        return 1;
    }

    return 0;
}

int unpack_file(FILE *stream, char *curr_URL) {
    char *csv_line = (char*)malloc(max_csv_line);
    char fname[MAX_FNAME];
    char lname[MAX_LNAME];
    char age_arr[MAX_AGE_LEN];
    unsigned int age;
    int r, flag = 0;

    while (1) {
        r = readline(csv_line, max_csv_line, stream);
        if (r == 1) {
            break;
        }

        //Readline worked
        if (r == 0) {
 
            r = unpack_row(fname, lname, age_arr, csv_line);

            //Unpack_row worked
            if (r == 0) {
                r = my_stoi(&age, age_arr);

                //my_stoi worked
                if (r == 0 && age < MAX_AGE) {
                    //update the globals
                    Age_counter[age]++;
                    if (Age_dict[age].fname == NULL) {
                        char *dict_fname = (char*)malloc(MAX_FNAME);
                        strcpy(dict_fname, fname);
                        Age_dict[age].fname = dict_fname;
                        char *dict_lname = (char*)malloc(MAX_LNAME);
                        strcpy(dict_lname, lname);
                        Age_dict[age].lname = dict_lname;
                    }
                }
            }
        }

        if (r != 0) {
            flag = 1;
        }
    }
    free(csv_line);
    return flag;
}


int main() {
    //initialize things
    FILE *URL_file, *csv_file;
    char curr_URL[MAX_URL_SIZE];
    char *csv_line = (char*)malloc(max_csv_line);
    int r;
    unsigned long n, s;
    unsigned int i, j, median, m_age;
    double mean, time_used;
    float median_f;
    clock_t start, end;

    start = clock();

    //Open the URL file
    URL_file = fopen(URLS_FNAME, "r");
    if (URL_file == NULL) {
        printf("could not find %s\n", URLS_FNAME);
        return 0;
    }

    for (i = 0; i < REPETITIONS; i++) {
        for (j = 0; j < 4; j++) {
            //set the file pointer to beginning
            rewind(URL_file);

            //Read a URL
            r = readline(curr_URL, MAX_URL_SIZE, URL_file);

            if (r == 1) {
                break;
            }

            //open the csv file
            csv_file = fopen(curr_URL, "r");

            //check the basics of this file
            if (csv_file) {
                readline(csv_line, max_csv_line, csv_file);
                r = strncmp(csv_line, CSV_HEADER, max_csv_line);
                if (r == 0) {
                    //Here we call some function that will unpack the entire file
                    r = unpack_file(csv_file, curr_URL);
                }

                //problem unpacking the CSV file
                if (r != 0) {
                    printf("Problem unpacking %s\n", curr_URL);
                }

                assert(fclose(csv_file) == 0);
            }

            //problem opening the file
            else {
                printf("Problem opening %s\n", curr_URL);
            }
        }
    }
    

    //free the URL and csv things
    free(csv_line);
    assert(fclose(URL_file) == 0);

    //analyze the globals
    n = s = 0;
    for (i = 0; i < MAX_AGE; i++) {
        n += Age_counter[i];
        s += Age_counter[i] * i;
    }
    
    mean = (double)s / (double)n;
    printf("mean is %f\n", mean);

    //odd case median
    if (n % 2) {
        int x = n / 2 + 1;
        for (i = 0; i < MAX_AGE; i++) {
            x -= Age_counter[i];
            if (x < 1) {
                m_age = i;
                break;
            }
        }
        printf("median is %d\n", m_age);
    }

    //even case median
    else{
        int x = n / 2;
        int y = x + 1;
        unsigned int m1, m2;
        //start m1 at an impossible age
        m1 = MAX_AGE;
        for (i = 0; i < MAX_AGE; i++) {
            x -= Age_counter[i];
            y -= Age_counter[i];
            if (x < 1 && m1 == MAX_AGE) {
                m1 = i;
            }
            if (y < 1) {
                m2 = i;
                break;
            }
        }

        m_age = m1;
        if (m1 == m2) {
            printf("median is %d\n", m_age);
        }
        else if ((m2 - m1) % 2) {
            median_f = (float)(m2 + m1) / 2;
            printf("median is %f\n", median_f);
        }
        else {
            median = (m2 + m1) / 2;
            printf("median is %d\n", median);
        }
    }
    printf("%s %s, %d\n", Age_dict[m_age].fname, Age_dict[m_age].lname, m_age);

    
    //free everything in Age_dict
    for (i = 0; i < MAX_AGE; i++) {
        if (Age_dict[i].fname) {
            free(Age_dict[i].fname);
            free(Age_dict[i].lname);
        }
    }

    end = clock();
    time_used = (double) (end - start) / CLOCKS_PER_SEC;

    printf("Total time used is %f\n", time_used);

    return 0;
}
