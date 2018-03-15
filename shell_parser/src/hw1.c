#include "hw1.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the program
 * and will return a unsigned short (2 bytes) that will contain the
 * information necessary for the proper execution of the program.
 *
 * IF -p is given but no (-r) ROWS or (-c) COLUMNS are specified this function
 * MUST set the lower bits to the default value of 10. If one or the other
 * (rows/columns) is specified then you MUST keep that value rather than assigning the default.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return Refer to homework document for the return value of this function.
 */


long double space;
char *buffer;
long double space1;
char *decode_char;
int decode_flag=0;
int buffer_number=0;
int write_flag=0;
int read_flag=0;





unsigned short validargs(int argc, char **argv) {
    unsigned short r=0x0000;
    if (*(*(argv+1)) == '-' && *(*(argv+1)+1) == 'h' && *(*(argv+1)+2) == '\000'){  //for -h
        r=0x8000;
        return r;
    }
    else if (*(*(argv+1)) == '-' && *(*(argv+1)+1) == 'p' && *(*(argv+1)+2) == '\000' && argc >= 2){  //for -p
        r=0x0000;
        if (*(*(argv+2)) == '-' && *(*(argv+2)+1) == 'e' && *(*(argv+2)+2) == '\000' && argc >= 3){  //for -p -e
            r+=0x0000;
        }else if(*(*(argv+2)) == '-' && *(*(argv+2)+1) == 'd' && *(*(argv+2)+2) == '\000' && argc >= 3){  //for -p -d
            r+=0x2000;
        }
        else{
            r=0x0;
            return r;
        }

        int i;
        int j;
        int row=10;
        int col=10;
        int flag_r, flag_c, flag_k;
        for (i=3;i<argc;i++){
            flag_r = 0;
            flag_c = 0;
            flag_k = 0;
            if (*(*(argv+i)) == '-' && *(*(argv+i)+1) == 'r' && *(*(argv+i)+2) == '\000' && flag_r == 0){  //for row
                flag_r = 1;
                i += 1;
                j=0;
                row=0;
                while(*(*(argv+i)+j)-'0'>=0 && *(*(argv+i)+j)-'0'<=9){
                    row=10*row + *(*(argv+i)+j)-'0';
                    j += 1;
                }
                if ((*(*(argv+i)+j)) != '\000' || row>15 || col<9){
                    r = 0x0;
                    return r;
                }
            }
            else if (*(*(argv+i)) == '-' && *(*(argv+i)+1) == 'c' && *(*(argv+i)+2) == '\000' && flag_c == 0){  //for col
                flag_c = 1;
                i += 1;
                j=0;
                col=0;
                while(*(*(argv+i)+j)-'0'>=0 && *(*(argv+i)+j)-'0'<=9){
                    col=10*col + *(*(argv+i)+j)-'0';
                    j += 1;
                }
                if ((*(*(argv+i)+j)) != '\000' || col>15 || col<9){
                    r = 0x0;
                    return r;
                }

            }
            else if (*(*(argv+i)) == '-' && *(*(argv+i)+1) == 'k' && *(*(argv+i)+2) == '\000' && flag_k == 0){  //for key
                flag_k = 1;
                i += 1;
                key = &(*(*(argv+i)));
                int ii=0;
                int jj=0;
                int kk=0;
                while (*(key+ii) != '\0'){   //key validation
                    jj=0;
                    kk=1;
                    while (*(key+ii) != *(polybius_alphabet+jj)){   //key in polybius_alphabet
                        if (*(polybius_alphabet+jj) == '\0'){
                            //printf("%s\n","not in polybius_alphabet");
                            r = 0x0;
                            return r;
                        }
                        jj++;
                    }

                    while (*(key+ii+kk)!= '\0'){         //key not repeat
                        if (*(key+ii) == *(key+ii+kk)){
                            r = 0x0;
                            //printf("%s\n","repeat");
                            return r;
                        }
                        kk++;
                    }
                    ii++;
                }


            }
            else {
                r= 0x0;
                return r;
            }
        }
        int k=0;
        while(*(polybius_alphabet+k)!='\0'){
            k += 1;
        }
        if (row*col < k){
            r = 0x0;
            return r;
        }
        r = r+16*row+col;

    }
    else if(*(*(argv+1)) == '-' && *(*(argv+1)+1) == 'f' && *(*(argv+1)+2) == '\000' && argc >= 2){   //for -f
        r=0x4000;
        if (*(*(argv+2)) == '-' && *(*(argv+2)+1) == 'e' && *(*(argv+2)+2) == '\000' && argc >= 3){  //for -f -e
            r+=0x0000;
        }else if(*(*(argv+2)) == '-' && *(*(argv+2)+1) == 'd' && *(*(argv+2)+2) == '\000' && argc >= 3){  //for -f -d
            r+=0x2000;
        }
        else{
            r=0x0;
            return r;
        }

        int i = 0;
        if(argc == 5){
            if(*(*(argv+3)) == '-' && *(*(argv+3)+1) == 'k' && *(*(argv+3)+2) == '\000'){   //for key
                key = &(*(*(argv+4)));
            }
            else{
                r=0x0;
                return r;
            }
            //printf("%s\n", key);
            int ii=0;
            int jj=0;
            int kk=0;
            while (*(key+ii) != '\0'){   //key validation
                jj=0;
                kk=1;
                while (*(key+ii) != *(fm_alphabet+jj)){   //key in fm_alphabet
                    if (*(fm_alphabet+jj) == '\0'){
                        //printf("%s\n","not in fm_alphabet");
                        r = 0x0;
                        return r;
                    }
                    jj++;
                }
                while (*(key+ii+kk)!= '\0'){         //key not repeat
                    if (*(key+ii) == *(key+ii+kk)){
                        r = 0x0000;
                        //printf("%s\n","repeat");
                        return r;
                    }
                    kk++;
                }
                ii++;
            }

        }
        else if(argc == 4 || argc > 5){
            r=0x0;
            return r;
        }
    }


    return r;
}


int print_buffer(){
    while (buffer_number >= 3){
        int ii;
        for (ii=0;ii<26;ii++){
            if (*(buffer+read_flag) == *(*(fractionated_table+ii))){
                read_flag++;
                if (read_flag == 8){
                    read_flag = 0;
                }
                if(*(buffer+read_flag) == *(*(fractionated_table+ii)+1)){
                    read_flag++;
                    if (read_flag == 8){
                        read_flag = 0;
                    }
                    if(*(buffer+read_flag) == *(*(fractionated_table+ii)+2)){
                        putchar(*(fm_key+ii));
                        //printf("buffer = %s and read_flag = %d and buffer_number = %d\n",buffer,read_flag, buffer_number);
                        buffer_number -= 3;
                        read_flag++;
                        if (read_flag == 8){
                            read_flag = 0;
                        }
                        break;
                    }
                    else{
                        read_flag--;
                        if (read_flag == -1){
                            read_flag = 7;
                        }
                        read_flag--;
                        if (read_flag == -1){
                            read_flag = 7;
                        }

                    }
                }
                else{
                    read_flag--;
                    if (read_flag == -1){
                            read_flag = 7;
                        }

                }
            }
        }
        if (ii==26){
            //printf("%s\n","Invalid input.");
            return 1;
            break;
        }
    }
    return 0;
}

void clean_buffer(){
    for (int i=0;i<9;i++){
        *(buffer+i) = '\0';
    }
    buffer_number=0;
    write_flag=0;
    read_flag=0;
}

void write_buffer(char temp){
    *(buffer+write_flag) = temp;
    write_flag++;
    if (write_flag == 8){
        write_flag = 0;
    }
    buffer_number++;
}

void print_buffer_decode(){
    int i;
    int j;
    for (i=0;i<89;i++){
        j=0;
        while(*(buffer+j)!='\0' && *(*(morse_table+i)+j)!='\0'){
            if (*(buffer+j)!=*(*(morse_table+i)+j))
                break;
            j++;
        }
        if(*(buffer+j) =='\0' && *(*(morse_table+i)+j) =='\0'&& *(*(morse_table+i)) != '\0'){
            putchar(i+33);
            break;
        }
    }
    if (i == 89){

    }
}