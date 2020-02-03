#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define TRUE 1
#define FALSE -1

#define DICTSIZE 4096                     /* allow 4096 entries in the dict  */
#define ENTRYSIZE 32

unsigned char dictionary[DICTSIZE][ENTRYSIZE];  /* of 30 chars max; the first byte */
          	                                /* is string length; index 0xFFF   */
                	                        /* will be reserved for padding    */
                        	                /* the last byte (if necessary)    */

// These are provided below
void encode(FILE *infil, FILE *outfil);
void decode(FILE *infil, FILE *outfil);
int read12(FILE *infil);
int write12(FILE *outfil, int int12);
void strip_lzw_ext(char *fname);
void flush12(FILE *outfil);
void add_to_dictionary_d(char current[], int length, int code);
void add_to_dictionary_e(char current[], int length);
int in_dictionary(char current[], int length);


int main(int argc, char *argv[]) {
   

 /* Checking for invalid or missing arguments */


        if(argc == 1){ // if not input file is given

                printf("Error: No input file specified!\n");

                exit(1);
        }


        if(argc == 2){ // if no encode/decode argument is given 

                printf("Invalid Usage, expected: RLE {%s} [e|d]\n", argv[1]);

                exit(4);
        }

        if (*argv[2] != 'd' &&  *argv[2] != 'e'){ // if the encode/decode arguent is not 'e' or 'd'

                printf("Invalid Usage, expected: LZW {%s} [e|d]\n", argv[1]);

                exit(4);
        }



        /* Opening file  */


        FILE* input = fopen(argv[1], "rb");

        if(input == NULL){ // if file DNE or can't be read

                printf("Read error: file not found or cannot be read\n");

                exit(2);

        }


     /* inputing the first 0-255 ASCII codes */


        int i = 0;

        while(i < 256){ 

                dictionary[i][0] = 1 + '0';	

        	dictionary[i][1] = i;

		i++;

        }

	
        /* Checking weather to encode or decode data */


	int size = strlen(argv[1]);
	
	char fname[size];

	strcpy(fname, argv[1]);

	if(*argv[2] == 'd' && (fname[size - 4] != '.' || fname[size - 3] != 'L' || fname[size - 2] != 'Z' || fname[size - 1] != 'W')){ // if you are decoding a file without a .LZW extention 
	
		printf("Error: File could not be decoded\n");

		exit(5);

	}
		
        if(*argv[2] == 'e'){ // adding the .LZW extension
	
		char fname[size + 4];
		
		strcpy(fname , argv[1]);

		fname[size] = '.';
		fname[size + 1] = 'L';
	 	fname[size + 2] = 'Z';
		fname[size + 3] = 'W';	 
		fname[size + 4] = '\0';

		FILE* output = fopen(fname, "wb");
                
		encode(input, output);
		
		fclose(output);
        }
	
        if(*argv[2] == 'd'){ // removing the .LZW extension		
		
		strip_lzw_ext(fname);
		
		FILE* output = fopen(fname, "wb");
                
		decode(input, output);

		fclose(output);
        }

        fclose(input);

        return 0;
 


	}

/*****************************************************************************/
/* encode() performs the Lempel Ziv Welch compression from the algorithm in  */
/* the assignment specification. The strings in the dictionary have to be    */
/* handled carefully since 0 may be a valid character in a string (we can't  */
/* use the standard C string handling functions, since they will interpret   */
/* the 0 as the end of string marker). Again, writing the codes is handled   */
/* by a separate function, just so I don't have to worry about writing 12    */
/* bit numbers inside this algorithm.                                        */


	void encode(FILE *input, FILE *output) {
                
	
		char current[31];

		int length = 1;
		
		current[0] = fgetc(input);

		int row = 256;

		int dictionary_code = current[0];


		while (current[0] != EOF){
					
		
			if(in_dictionary(current, length) != FALSE){ // if it is in the dictionary
			
				dictionary_code = in_dictionary(current, length); 

				current[length] = fgetc(input);

				length++;

			}

			else {
				
				write12(output, dictionary_code); // output the dictionary code to the file
	
				if(length < 32 && row < 4096){	

					add_to_dictionary_e(current, length);
					
					row++;

				}

				int k = 1;

				current[0] = current[length - 1]; 
		

				while (k < length){

					current[k] = '\0';
					
					k++;
				}
				
				length = 1;
		
			}
					 			
		}    
 			
			
		write12(output, 4095); // add padding marker
		
		flush12(output); // flush out last value from write12

		exit(0);	
	
	
	}	


/*****************************************************************************/
/* decode() performs the Lempel Ziv Welch decompression from the algorithm   */
/* in the assignment specification.                                          */


	void decode(FILE *input, FILE *output) {


		int code = read12(input);

		int row = 256;

		int i = 0;

		char decoded[31];

		char entry[31];

		int length;
	

		while(i < 32){
		
			decoded[i] = '\0';

			entry[i] = '\0';
			
			i++;

		}

		decoded[0] = code;	

		fputc(decoded[0], output); // output first character		

		code = read12(input); 

		while(code!= 0x0FFF){
		
			int entry_length = 0;

			i = 0;

                	while(decoded[i] != '\0'){ // copy decoded string (W) into the next entry
			
                		entry[i] = decoded[i];
		
                        	i++;

                        	entry_length++;

                	}

		 	if (dictionary[code][0] != '\0'){ // if it is in the dictionary

				int n  = 1;

				length = dictionary[code][0] - 48;

				while(n <= length){ // output the string at dictionary code
			
					fputc(dictionary[code][n], output);

					n++;
				}
				
		        	entry[i] = dictionary[code][1]; // add character in the dictionary at row code and column 1
			
				entry_length++;

				if(entry_length < 32 && row < 4095){
			
                        		add_to_dictionary_d(entry,entry_length, row);

                       			row++;

                       		 }

			}

			else{
		
				entry[entry_length] = decoded[0]; // add the first character of decoded (W) to the entry
	
				entry_length++;

                	        if(entry_length < 32 && row < 4095){

					add_to_dictionary_d(entry, entry_length, code);

					row++;

				}	
		
				i = 1;
		

				while(i <= entry_length){  // output entry
		
                                	fputc(entry[i-1], output);
		
                                	i++;
                       		}
			
			}	
			        
			i = 0;


        	        while(decoded[i] != '\0'){

                		decoded[i] = '\0';
			
				i++;
               		}   

               		i = 1;

               		while(i <= length){ // set decoded (W) to the string added to the dictionary
                       
                     		decoded[i-1] = dictionary[code][i];

                      		i++;
			}
				
			code = read12(input);
			      
		}	
	
		exit(0);
		
	}


/*****************************************************************************/
/* read12() handles the complexities of reading 12 bit numbers from a file.  */
/* It is the simple counterpart of write12(). Like write12(), read12() uses  */
/* static variables. The function reads two 12 bit numbers at a time, but    */
/* only returns one of them. It stores the second in a static variable to be */
/* returned the next time read12() is called.                                */
int read12(FILE *infil)
{
 static int number1 = -1, number2 = -1;
 unsigned char hi8, lo4hi4, lo8;
 int retval;

 if(number2 != -1)                        /* there is a stored number from   */
    {                                     /* last call to read12() so just   */
     retval = number2;                    /* return the number without doing */
     number2 = -1;                        /* any reading                     */
    }
 else                                     /* if there is no number stored    */
    {
     if(fread(&hi8, 1, 1, infil) != 1)    /* read three bytes (2 12 bit nums)*/
        return(-1);
     if(fread(&lo4hi4, 1, 1, infil) != 1)
        return(-1);
     if(fread(&lo8, 1, 1, infil) != 1)
        return(-1);

     number1 = hi8 * 0x10;                /* move hi8 4 bits left            */
     number1 = number1 + (lo4hi4 / 0x10); /* add hi 4 bits of middle byte    */

     number2 = (lo4hi4 % 0x10) * 0x0100;  /* move lo 4 bits of middle byte   */
                                          /* 8 bits to the left              */
     number2 = number2 + lo8;             /* add lo byte                     */

     retval = number1;
    }

 return(retval);
}

/*****************************************************************************/
/* write12() handles the complexities of writing 12 bit numbers to file so I */
/* don't have to mess up the LZW algorithm. It uses "static" variables. In a */
/* C function, if a variable is declared static, it remembers its value from */
/* one call to the next. You could use global variables to do the same thing */
/* but it wouldn't be quite as clean. Here's how the function works: it has  */
/* two static integers: number1 and number2 which are set to -1 if they do   */
/* not contain a number waiting to be written. When the function is called   */
/* with an integer to write, if there are no numbers already waiting to be   */
/* written, it simply stores the number in number1 and returns. If there is  */
/* a number waiting to be written, the function writes out the number that   */
/* is waiting and the new number as two 12 bit numbers (3 bytes total).      */
int write12(FILE *outfil, int int12)
{
 static int number1 = -1, number2 = -1;
 unsigned char hi8, lo4hi4, lo8;
 unsigned long bignum;

 if(number1 == -1)                         /* no numbers waiting             */
    {
     number1 = int12;                      /* save the number for next time  */
     return(0);                            /* actually wrote 0 bytes         */
    }

 if(int12 == -1)                           /* flush the last number and put  */
    number2 = 0x0FFF;                      /* padding at end                 */
 else
    number2 = int12;

 bignum = number1 * 0x1000;                /* move number1 12 bits left      */
 bignum = bignum + number2;                /* put number2 in lower 12 bits   */

 hi8 = (unsigned char) (bignum / 0x10000);                     /* bits 16-23 */
 lo4hi4 = (unsigned char) ((bignum % 0x10000) / 0x0100);       /* bits  8-15 */
 lo8 = (unsigned char) (bignum % 0x0100);                      /* bits  0-7  */

 fwrite(&hi8, 1, 1, outfil);               /* write the bytes one at a time  */
 fwrite(&lo4hi4, 1, 1, outfil);
 fwrite(&lo8, 1, 1, outfil);

 number1 = -1;                             /* no bytes waiting any more      */
 number2 = -1;

 return(3);                                /* wrote 3 bytes                  */
}

/** Write out the remaining partial codes */
void flush12(FILE *outfil)
{
 write12(outfil, -1);                      /* -1 tells write12() to write    */
}                                          /* the number in waiting          */

/** Remove the ".LZW" extension from a filename */
void strip_lzw_ext(char *fname)
{
    char *end = fname + strlen(fname);

    while (end > fname && *end != '.' && *end != '\\' && *end != '/') {
        --end;
    }
    if ((end > fname && *end == '.') &&
        (*(end - 1) != '\\' && *(end - 1) != '/')) {
        *end = '\0';
    }
}


/*****************************************************************************/
/* This function will take the entry, it's length and the row and add the    */
/* at the row provided (starting with the entry's length). This function is  */
/* used in the decode function.						     */

	void add_to_dictionary_d(char entry[], int length, int row){
	
		dictionary[row][0] = length + '0';

		int k = 1;
		
		while(k <= length){
	
		dictionary[row][k] = entry[k-1];

		k++;

		}

	}


/*****************************************************************************/
/* This function will take the current entry, and it's length and add it to  */
/* the dictionary at the next open row (starting with the entry's length).   */
/* This function is used in the encode function.  			     */

	void add_to_dictionary_e(char current[], int length){
		
	       
		int i = 255;

                int n = 0;

		int k = 1;

                while(dictionary[i][0] != '\0'){
               
                	i++;		

                }
		
		dictionary[i][0] = length + '0';
		
		while(k <= length){	
						
			dictionary[i][k] = current[n];
			
			k++;
			
			n++;

		}

	}	


/*****************************************************************************/
/* This function will take the current entry, and it's length and check if   */
/* that entry is in the dictionary, returning the row where it is located or */
/* FALSE is it is not in the dictionary. This function is used in the encode */
/* function.								     */

	int in_dictionary(char current[], int length){
		
		
		if(length == 1){

			int index = current[0];
			
			return index;
		
		}

		int i = 255;

		int k = 0;

		int n = 2;

		char char_length = length +'0';
	       		
		while(i < 4096){
		
			if(dictionary[i][0] == '\0'){

				return FALSE;
			}	
	
			if(dictionary[i][0] == char_length){
				
				n = 1;

				while(n <= length){
					
                			if(dictionary[i][n] != current[n-1]){
	
                                                break;
                                        }

                                        if(n == length){

                                                return i;
						
                                        }

                                        n++;

                			}
				
				}
			i++;

			}
	

	}



