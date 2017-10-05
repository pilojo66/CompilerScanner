/* File name: buffer.c
** Compiler: MS Visual Studio 2015
** Author: John Pilon, 040822687
** Course: CST8152-Compilers, Lab 012
** Date: September 16th, 2017
** Professor: Sv. Ranev
** Purpose: Definitions of all functions declared in buffer.h; All functions relate to the tasks of a buffer
** Function list:
b_allocate()
b_addc()
b_clear()
b_free()
b_isfull()
b_limit()
b_capacity()
b_mark()
b_mode()
b_incfactor()
b_load()
b_isempty()
b_eob()
b_getc()
b_print()
b_compact()
b_rflag()
b_retract()
b_reset()
b_getcoffset()
b_rewind()
b_location()
*/

#include "buffer.h"

/* Purpose: Allocate memory for a buffer
** Author: John Pilon
** Version: 1.0
** Called functions: malloc(), calloc(), free()
** Parameters: init_capacity(in bytes): 0..(SHRT_MAX-1); inc_factor: 0..100 for mode -1(percentage), 0..255 for mode 1(in bytes); o_mode: -1..1
** Return Value: Fully allocated Buffer*
** Algorithm: Allocate memory for the buffer descriptor and char array, check o_mode for type, return buffer descriptor
*/
Buffer *b_allocate(short init_capacity, char inc_factor, char o_mode) {
	Buffer *newBuffer; /* new buffer to be created */
	char *charArry; /* char array for the buffer */

					/* If it is a fixed-size of 0, it cannot be created */
	if (o_mode == 'f' && init_capacity == 0) {
		return NULL;
	}

	/*Check for negative values*/
	if (init_capacity < 0 || init_capacity == SHRT_MAX) return NULL;

	/* allocating the size of a buffer to the pointer */
	newBuffer = (pBuffer)calloc(1, sizeof(Buffer));
	if (newBuffer == NULL) return NULL;

	/* allocating the initial capacity of the char array */
	charArry = (char*)malloc(sizeof(char)*init_capacity);

	/* switch the buffer to fixed mode if the inc_factor is 0 */
	if (inc_factor == 0) o_mode = 'f';

	/*determine operating mode*/
	switch (o_mode) {
		/* initialize fixed mode */
	case 'f':
		newBuffer->mode = FIXED_MODE;
		newBuffer->inc_factor = 0;
		break;

		/* initialize additive mode */
	case 'a':
		if ((unsigned char)inc_factor > 0) {
			newBuffer->mode = ADDITIVE_MODE;
			newBuffer->inc_factor = inc_factor;
		}
		/* Program should never reach this return! */
		else {
			free(newBuffer);
			free(charArry);
			return NULL;
		}
		break;

		/* initialize multiplicative mode */
	case 'm':
		if (inc_factor <= MAX_M_MODE_INC && inc_factor > 0) {
			newBuffer->mode = MULTIPLICATIVE_MODE;
			newBuffer->inc_factor = inc_factor;
		}
		else {
			free(newBuffer);
			free(charArry);
			return NULL;
		}
		break;

		/* Incorrect mode given */
	default:
		free(newBuffer);
		free(charArry);
		return NULL;
	}

	/* assign remaining structure members */
	newBuffer->cb_head = charArry;
	newBuffer->capacity = init_capacity;

	return newBuffer;
}

/* Purpose: Adds a character to the buffer
** Author: John Pilon
** Version: 1.0
** Called functions: realloc()
** Parameters: pBD: a valid Buffer pointer, symbol: any char
** Return Value: Buffer pointer
** Algorithm: Check if there's room for a symbol, if not, reallocate by inc_factor and o_mode, then add symbol
*/
pBuffer b_addc(pBuffer const pBD, char symbol) {
	unsigned short newCapacity = 0; /* unsigned capacity variable to check for overflow */
	char *tempStorage = NULL;	/* used to check if the memory address changed */

								/* check for valid parameters */
	if (pBD == NULL) return NULL;

	/* Check if there's room for a symbol */
	if (pBD->addc_offset < pBD->capacity) {
		pBD->cb_head[pBD->addc_offset++] = symbol;
	}
	/* Check for o_mode */
	else if (pBD->mode != FIXED_MODE) {
		if (pBD->mode == ADDITIVE_MODE) {
			/* Check if capacity will overflow */
			newCapacity = pBD->capacity + (unsigned char)pBD->inc_factor;
			if (newCapacity > SHRT_MAX - 1) return NULL;
			tempStorage = (char*)realloc(pBD->cb_head, sizeof(char)*((unsigned char)pBD->inc_factor + pBD->capacity)); /* assign array new capacity */
		}
		else {
			/* Check if capacity is already maximum */
			if (pBD->capacity == (SHRT_MAX - 1)) return NULL;
			newCapacity = (unsigned short)(pBD->capacity + ((((SHRT_MAX - 1) - pBD->capacity) * pBD->inc_factor) / 100));
			/* Fix for calculation resulting in 0 growth when there is still memory available to be used */
			if (newCapacity == pBD->capacity) newCapacity = SHRT_MAX - 1;
			/* Check if capacity overflowed */
			if (newCapacity >= SHRT_MAX - 1) newCapacity = SHRT_MAX - 1;
			tempStorage = (char*)realloc(pBD->cb_head, sizeof(char)*newCapacity); /* assign array with new capacity */
		}

		/* Check if array was reallocated successfully */
		if (tempStorage == NULL) return NULL;

		/* set reallocation flag if the memory location changed */
		if (pBD->cb_head != tempStorage) {
			pBD->r_flag = SET_R_FLAG;
		}

		pBD->cb_head = tempStorage;
		pBD->capacity = newCapacity;

		pBD->cb_head[pBD->addc_offset++] = symbol;

	}
	else return NULL;

	return pBD;
}

/* Purpose: clears all relevant values in the buffer
** Author: John Pilon
** Version: 1.0
** Called functions: N/A
** Parameters: pBD: A valid Buffer pointer
** Return value: int: -1 for fail, 0 for success
*/
int b_clear(Buffer *const pBD) {
	if (pBD != NULL) {
		pBD->addc_offset = BUFFER_START;
		pBD->getc_offset = BUFFER_START;
		pBD->markc_offset = BUFFER_START;
		pBD->eob = !EOB;
		return SUCCESS;
	}
	else return RT_FAIL1;
}

/* Purpose: Frees all memory on the heap
** Author: John Pilon
** Version 1.0
** Called functions: free()
** Parameters: pBD: A valid Buffer pointer
** Return value: none
*/
void b_free(Buffer *const pBD) {
	char *cb_head; /* temporary pointer to help free memory */
	if (pBD != NULL) {
		cb_head = pBD->cb_head;
		free(cb_head);
		free(pBD);
	}
	cb_head = NULL; /* Free dangling pointer */
}

/* Purpose: Checks if the buffer is full
** Author: John Pilon
** Version: 1.0
** Called functions: N/A
** Parameters: pBD: A valid Buffer pointer
** Return value: int: -1 on fail, 0 on false, 1 on true
** Special notes: If B_FULL is defined, a macro is used instead
*/
#ifndef B_FULL
int b_isfull(Buffer *const pBD) {
	return pBD == NULL ? RT_FAIL1 : pBD->eob;
}
#endif

/* Purpose: Returns the current number of characters in the buffer
** Author: John Pilon
** Version 1.0
** Called functions: N/A
** Parameters: pBD: A valid Buffer pointer
** Return value: short: -1 on fail, addc_offset otherwise
*/
short b_limit(Buffer *const pBD) {
	return pBD == NULL ? RT_FAIL1 : pBD->addc_offset;
}

/* Purpose: Returns the maximum amount of possible characters in the buffer
** Author: John Pilon
** Version: 1.0
** Called functions: N/A
** Parameters: pBD: A valid Buffer pointer
** Return value: short: -1 on fail, capacity otherwise
*/
short b_capacity(Buffer *const pBD) {
	return pBD == NULL ? RT_FAIL1 : pBD->capacity;
}

/* Purpose: Returns markc_offset
** Author: John Pilon
** Version 1.0
** Called functions: N/A
** Parameters: pBD: A valid Buffer pointer, mark: 0..addc_offset
** Return value: short: value of markc_offset
*/
short b_mark(Buffer *const pBD, short mark) {
	if (mark > pBD->addc_offset || mark < BUFFER_START) {
		return RT_FAIL1;
	}
	pBD->markc_offset = mark;
	return pBD->markc_offset;
}

/* Purpose: Returns operating mode
** Author: John Pilon
** Version 1.0
** Called functions: N/A
** Parameters: pBD: A valid Buffer pointer
** Return value: int: value of mode
*/
int b_mode(Buffer *const pBD) {
	return pBD == NULL ? RT_FAIL1 : pBD->mode;
}

/* Purpose: Returns inc_factor
** Author: John Pilon
** Version: 1.0
** Called functions: N/A
** Parameters: pBD: A valid Buffer pointer
** Return Value: size_t: value of inc_factor
*/
size_t b_incfactor(Buffer *const pBD) {
	return pBD == NULL ? RT_FAIL256 : (unsigned char)pBD->inc_factor;
}


/* Purpose: loads a file into memory
** Author: John Pilon
** Version: 1.0
** Called functions: fgetc(), feof(), b_addc(), fclose()
** Parameters: fi: A valid FILE pointer, pBD: A valid Buffer pointer
** Return value: int: number of characters added to the buffer
** Algorithm: loop until end of file is reached. Add a character on every iteration
*/
int b_load(FILE * const fi, Buffer * const pBD)
{
	char c = 0; /* temporary char before adding to buffer */
	int numAdded = 0; /* keeps track of the number of characters added to the buffer */
					  /* Loop until EOF is reached */
	while (1) {
		c = (char)fgetc(fi);
		/* Check for eof */
		if (feof(fi)) break;
		/* Check if add was successful */
		if (b_addc(pBD, c) == NULL)
		{
			fclose(fi);
			return LOAD_FAIL;
		}
		numAdded++;
	}
	fclose(fi);
	return numAdded;
}

/* Purpose: Checks if buffer is empty
** Author: John Pilon
** Version: 1.0
** Called functions: N/A
** Parameters: pBD: A valid Buffer pointer
** Return value: int: -1 on fail, 0 on false, 1 on true
*/
int b_isempty(Buffer *const pBD) {
	if (pBD == NULL) return RT_FAIL1;
	else if (pBD->addc_offset > BUFFER_START) return !EMPTY;
	else return EMPTY;
}

/* Purpose: Returns eob
** Author: John Pilon
** Version: 1.0
** Called functions: none
** Parameters: pBD: A valid Buffer pointer
** Return value: int: -1 on fail, eob otherwise
*/
int b_eob(Buffer *const pBD) {
	return pBD == NULL ? RT_FAIL1 : pBD->eob;
}

/* Purpose: Returns character at getc_offset
** Author: John Pilon
** Version: 1.0
** Called functions: N/A
** Parameters: pBD: A valid Buffer pointer
** Return value: char: the char at getc_offset
*/
char b_getc(Buffer *const pBD) {
	if (pBD == NULL) return RT_FAIL2;
	else {
		/* set eob flag if the getc_offset reaches the end of the buffer */
		if (pBD->getc_offset >= pBD->addc_offset) {
			pBD->eob = EOB;
			return RT_FAIL1;
		}
		/* remove eob flag if the buffer has grown */
		else {
			pBD->eob = !EOB;
			return pBD->cb_head[pBD->getc_offset++];
		}
	}
}

/* Purpose: Prints all contents of the buffer
** Author: John Pilon
** Version: 1.0
** Called functions: b_getc(), b_eob()
** Parameters: pBD: A valid Buffer pointer
** Return value: int: number of printed characters
** Algorithm: Check if the buffer is empty, if not, loop until the end of buffer while printing the next character every iteration
*/
int b_print(Buffer *const pBD) {
	int numPrinted = 0; /* Number of characters printed */
	char c; /* temporary char for comparison */
	if (pBD == NULL) return RT_FAIL1;
	/* Check for empty buffer */
	else if (pBD->addc_offset == BUFFER_START) {
		printf("Empty buffer\n");
		return EMPTY;
	}
	else {
		/* Loop until end of buffer and print contents */
		while (1) {
			c = b_getc(pBD);
			if (b_eob(pBD)) break;
			printf("%c", c);
			numPrinted++;
		}
		printf("\n");
	}
	return numPrinted;
}

/* Purpose: Fit the array to the exact number of bytes needed
** Author: John Pilon
** Version: 1.0
** Called functions: realloc()
** Parameters: pBD: A valid Buffer pointer, symbol: any char
** Return Value: A Buffer pointer with the new capacity
** Algorithm: Reallocate array to its current size + 1, add symbol
*/
Buffer *b_compact(Buffer *const pBD, char symbol) {
	char *tempStorage; /* temporary char array to be assigned */

	if (pBD == NULL) return NULL;

	tempStorage = (char*)realloc(pBD->cb_head, pBD->addc_offset + 1);
	if (tempStorage == NULL) return NULL;

	/* Set reallocation flag if the memory location has moved */
	if (tempStorage != pBD->cb_head) {
		pBD->r_flag = SET_R_FLAG;
	}

	pBD->cb_head = tempStorage;
	pBD->capacity = pBD->addc_offset + 1;
	pBD->cb_head[pBD->addc_offset++] = symbol;

	return pBD;
}

/* Purpose: Return the r_flag
** Author: John Pilon
** Version: 1.0
** Called functions: N/A
** Parameters: pBD: A valid Buffer pointer
** Return Value: char: value of r_flag
*/
char b_rflag(Buffer *const pBD) {
	return pBD == NULL ? RT_FAIL1 : pBD->r_flag;
}

/* Purpose: move the getc_offset back one space
** Author: John Pilon
** Version: 1.0
** Called functions: N/A
** Parameters: pBD: A valid Buffer pointer
** Return Value: short: value of getc_offset after move
*/
short b_retract(Buffer *const pBD) {
	if (pBD == NULL) return RT_FAIL1;
	else if (pBD->getc_offset == BUFFER_START || pBD->getc_offset == pBD->markc_offset) return pBD->getc_offset;
	else {
		pBD->getc_offset--;
		return pBD->getc_offset;
	}
}

/* Purpose: reset the getc_offset to the markc_offset
** Author: John Pilon
** Version: 1.0
** Called functions: N/A
** Parameters: pBD: A valid Buffer pointer
** Return Value: short: value of getc_offset after move
*/
short b_reset(Buffer *const pBD) {
	if (pBD == NULL) return RT_FAIL1;
	else {
		pBD->getc_offset = pBD->markc_offset;
	}
	return pBD->getc_offset;
}

/* Purpose: return getc_offset
** Author: John Pilon
** Version: 1.0
** Called functions: N/A
** Parameters: pBD: A valid Buffer pointer
** Return Value: short: value of getc_offset
*/
short b_getcoffset(Buffer *const pBD) {
	return pBD == NULL ? RT_FAIL1 : pBD->getc_offset;
}

/* Purpose: reset getc_offset and markc_offset to beginning of buffer
** Author: John Pilon
** Version: 1.0
** Called functions: N/A
** Parameters: pBD: A valid Buffer pointer
** Return value: int: -1 on fail, 0 on success
*/
int b_rewind(Buffer *const pBD) {
	if (pBD == NULL) return RT_FAIL1;
	else {
		pBD->getc_offset = BUFFER_START;
		pBD->markc_offset = BUFFER_START;
	}
	return SUCCESS;
}

/* Purpose: return the char at the location of loc_offset
** Author: John Pilon
** Version: 1.0
** Called functions: N/A
** Parameters: pBD: A valid Buffer pointer, loc_offset(in elements):  a value less than addc_offset
** Return value: char: value of cb_head at loc_offset
*/
char *b_location(Buffer *const pBD, short loc_offset) {
	if (pBD == NULL || pBD->cb_head == NULL) return NULL;

	if (loc_offset < 0 || loc_offset >= pBD->addc_offset) return NULL;

	return &(pBD->cb_head[loc_offset]);
}