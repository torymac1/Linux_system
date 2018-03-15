#include <stdlib.h>

#include "hw1.h"
#include "debug.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

int main(int argc, char **argv)
{
    if (argc == 1){
        USAGE(*argv, EXIT_FAILURE);
        return EXIT_FAILURE;
    }

    unsigned short mode;

    mode = validargs(argc, argv);

    debug("Mode: 0x%X", mode);

    if(mode & 0x8000) {
        USAGE(*argv, EXIT_SUCCESS);
        return EXIT_FAILURE;
    }
    if((mode & 0xFFFF) == 0x0000){
        USAGE(*argv, EXIT_FAILURE);
        return EXIT_FAILURE;
    }
    if((mode & 0xF000) == 0x0000 || (mode & 0xF000) == 0x2000){   //-p -e or -p -d
        //-p -e
        int row = (mode & 0x00F0)/16;  //get row and col
        int col = mode & 0x000F;
        if (key == NULL){          //no key
            int i=0;
            while(*(polybius_alphabet+i) != '\0'){
                *(polybius_table+i) = *(polybius_alphabet+i);
                i++;
            }
            *(polybius_table+i)='\0';
        }
        else{                    //insert key to polybius_table
            int i=0;
            int j=0;
            int k=0;
            while(*(key+i) != '\0'){
                *(polybius_table+i) = *(key+i);
                i++;
            }
            int key_len=i;
            int flag=0;
            while(*(polybius_alphabet+j) != '\0'){     //insert polybius_alphabet to polybius_table
                flag = 0;
                for (k=0;k<key_len;k++){
                    if(*(polybius_alphabet+j) == *(key+k)){
                        flag = 1;
                        break;
                    }
                }
                if(flag == 0){
                    *(polybius_table+i)=*(polybius_alphabet+j);
                    i++;
                }
                j++;

            }
        }

        if ((mode & 0xF000) == 0x0000){   //encode
            char c;
            int i;
            int flag;
            while ((c=getchar()) != EOF){
                i = 0;
                flag = 0;
                while(1){
                    if (c == *(polybius_table+i) || c =='\n' || c==' ' || c=='\t' ){
                        flag = 1;
                        break;
                    }
                    i++;
                }
                if (flag == 1){
                    if(c==' ' || c == '\t' || c == '\n'){
                        putchar(c);
                    }
                    else{
                        int r = i/col;
                        int c = i - r*col;
                        printf("%X%X", r, c);
                    }
                }
            }
        }

        else if((mode & 0xF000) == 0x2000){
            char temp;
            int r;
            int c;
            int k;
            while((temp=getchar()) != EOF){
                if(temp == '\n' || temp==' ' || temp=='\t' ){
                    putchar(temp);
                }
                else{
                    if(48 <= temp && temp <= 57){
                        r = temp - 48;
                    }
                    else{
                        r = temp -65+10;
                    }
                    temp=getchar();
                    if(48 <= temp && temp <= 57){
                        c = temp - 48;
                    }
                    else{
                        c = temp -65+10;
                    }
                    if(temp == '\n' || temp==' ' || temp=='\t' ){
                        //printf("%d\n",c);
                        putchar(c);
                        continue;
                    }
                    k = r*col+c;
                    putchar(*(polybius_table+k));
                }
            }
        }
    }

    else if((mode & 0xF000) == 0x4000 || (mode & 0xF000) == 0x6000){    //for -f
        if (key == NULL){             //no key
            int i;
            for (i=0;i<26;i++){
                *(fm_key+i)=*(fm_alphabet+i);
            }
        }
        else{
            int i=0;
            int j=0;
            int k=0;
            while(*(key+i) != '\0'){
                *(fm_key+i) = *(key+i);
                i++;
            }
            int key_len=i;
            int flag=0;
            while(*(fm_alphabet+j) != '\0'){     //insert key to fm_table
                flag = 0;
                for (k=0;k<key_len;k++){
                    if(*(fm_alphabet+j) == *(key+k)){
                        flag = 1;
                        break;
                    }
                }
                if(flag == 0){
                    *(fm_key+i)=*(fm_alphabet+j);
                    i++;
                }
                j++;
            }
        }

        if ((mode & 0xF000) == 0x4000){     //encode
            char temp;
            char pre = '?';
            int i;
            int space_flag = 0;   //flag_space = 1 means pre-character is not whitespace
            buffer = (char*)&space;
            *(buffer+8) = '\0';
            while((temp=getchar()) != EOF){
                if (temp == ' ' || temp == '\n' || temp == '\t'){
                    if (pre != ' ' && pre != '\n' && temp != '\t'){
                        write_buffer('x');
                        if(print_buffer() == 1){
                            return EXIT_FAILURE;
                        }
                    }
                    if (temp == '\n'){
                        printf("%s","\n");
                        clean_buffer();
                        char pre = '?';
                    }
                }
                else if(temp>0x7A || temp<0x21 || *(morse_table+temp-0x21) == '\0'){
                    return EXIT_FAILURE;
                }
                else{
                    int j=0;
                    while(*(*(morse_table+temp-33)+j)!='\0'){
                        write_buffer(*(*(morse_table+temp-33)+j));
                        j++;
                        if(print_buffer() == 1){
                            return EXIT_FAILURE;
                        }
                    }
                    write_buffer('x');
                    if(print_buffer() == 1){
                        return EXIT_FAILURE;
                    }
                }
                pre = temp;
            }
        }
        if ((mode & 0xF000) == 0x6000){    //decode
            char temp;
            char next = '?';
            char pre = '?';
            int newline_flag = 0;
            buffer = (char*)&space;
            *(buffer+8) = '\0';
            int ii;
            temp = getchar();
            while(temp != EOF){
                next = getchar();
                if(next == '\n'){
                    newline_flag = 1;
                }
                else{
                    newline_flag = 0;
                }
                if (temp == '\n'){
                    if (*buffer!='\0'){
                        print_buffer_decode();
                    }
                    clean_buffer();
                    printf("%s","\n");
                    temp = next;
                    continue;
                }
                for(ii=0;ii<26;ii++){
                    if(temp == *(fm_key+ii)){
                        break;
                    }
                }
                int j;
                for(j=0;j<3;j++){
                    if(*(*(fractionated_table+ii)+j) == 'x'){
                        if(pre == 'x' && newline_flag == 0){
                            printf("%s"," ");
                        }
                        else{
                            write_buffer('\0');
                            print_buffer_decode();
                            clean_buffer();
                        }
                        pre = 'x';
                    }
                    else{
                        pre = *(*(fractionated_table+ii)+j);
                        write_buffer(*(*(fractionated_table+ii)+j));
                    }

                }
                temp = next;
            }
        }
    }
    return EXIT_SUCCESS;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */