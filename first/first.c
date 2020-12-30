#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

unsigned long long readCount = 0;
unsigned long long writeCount = 0;
unsigned long long hitCount = 0;
unsigned long long missCount = 0;



void incrementReadCount()
{
    readCount++;
}
void incrementWriteCount()
{
    writeCount++;
}
void incrementHitCount()
{
    hitCount++;
}
void incrementMissCount()
{
    missCount++;
}



// node struct and allocate

struct node
{
    int lineIndex;
    struct node* next;
};

struct node* allocate(int num)
{
    struct node* temp = malloc(sizeof(struct node));
    temp -> lineIndex = num;
    temp -> next = 0;
    return temp;
}



// set struct and allocate

struct set
{
    long long tag[512];  
    int valid[512];
};

struct set* setAllocate()
{
    struct set* s = malloc(sizeof(struct set));

    for (int i = 0; i < 512; i++)
    {
        s -> tag[i] = -1;
        s -> valid[i] = 0;
    }

    return s;

}












long long hexToNum (char* hex)
{

    long long dec = (long long)(strtol(hex, NULL, 0));

    return dec;
}


void convertNumToBin (int* array, long long inp)
{
    long long num = inp;

    while (num > 0)
    {
        int pwr = 0;

        while (num - (long long)(pow(2, pwr)) > 0)
        {
            pwr++;
        }

        int index;

        if (num - (long long)(pow(2, pwr)) == 0)
        {
            index = pwr;
        }
        else
        {
            index = pwr - 1;
        }
        

        array[index] = 1;

        num = num - (long long)(pow(2, index));

    }
}




int findBlockOffsetBits (int blockSize)
{
    int blockOffsetBits = 0;
    while ((long long)(pow(2, blockOffsetBits)) <= blockSize)
    {
        if ((long long)(pow(2, blockOffsetBits)) == blockSize)
        {
            break;
        }

        blockOffsetBits++;
    }

    return blockOffsetBits;
}

int findSetIndexBits (int numOfLines)
{
    int setIndexBits = 0;
    while ((long long)(pow(2, setIndexBits)) <= numOfLines)
    {
        if ((long long)(pow(2, setIndexBits)) == numOfLines)
        {
            break;
        }

        setIndexBits++;
    }

    return setIndexBits;
}



long long binToDec(int* storage, int startIndex, int endIndex)
{
    long long sum = 0;

    int index = 0;

    for (int i = startIndex; i <= endIndex; i++)
    {
        if (storage[i] == 1)
        {
            sum = sum + ((long long)(pow(2, index)));
        }

        index++;
    }

    return sum;
}




struct node* updateList (struct node* head, int replacePolicy, int lineNumber, int evict)       // evict = 0 is no eviction, evict = 1 is eviction
{
    if (head == 0)
    {
        struct node* temp = allocate(lineNumber);
        return temp;
    }

    struct node* ptr = head;
    
    int inList = 0;

    while (ptr != 0)
    {
        if(ptr -> lineIndex == lineNumber)
        {
            inList = 1;
            break;
        }

        ptr = ptr -> next;
    }

    if (inList == 0)
    {
        struct node* temp = allocate(lineNumber);

        temp -> next = head;

        return temp;
    }


    
    if (replacePolicy == 1)     // FIFO
    {
        if (evict == 0)
        {
            return head;
        }
        else
        {
            if (head -> lineIndex == lineNumber)
            {
                return head;
            }
            else
            {
                ptr = head;
                struct node* prev = head;

                while (ptr != 0)
                {
                    ptr = ptr -> next;
                    if (ptr -> lineIndex == lineNumber)
                    {
                        prev -> next = ptr -> next;
                        ptr -> next = head;
                        return ptr;
                    }
                    prev = prev -> next;
                }

                return ptr;
            }
        }
    }
    else                        // LRU
    {
        if (head -> lineIndex == lineNumber)
        {
           return head;
        }
        else
        {
            ptr = head;
            struct node* prev = head;

            while (ptr != 0)
            {
                ptr = ptr -> next;
                if (ptr -> lineIndex == lineNumber)
                {
                    prev -> next = ptr -> next;
                    ptr -> next = head;
                    return ptr;
                }
                prev = prev -> next;
            }

            return ptr;
        }
    }
}








// cache read method


void reqRead(struct set* cache[], int numOfLines, int numOfSets, int blockSize, char* hex, int assocType, int replacePolicy, struct node* list[])
{

    // Step 1: find block offset and set index bit size

    int blockOffsetBits = findBlockOffsetBits(blockSize);

    int setIndexBits = findSetIndexBits(numOfSets);

    


    // Step 2: convert the hex number into a binary form, then determine the
    //         exact values of the offset, set index, and tag given binary
    //         form.


    int binStorage[500];
    
    for (int i = 0; i < 500; i++)
    {
        binStorage[i] = 0;
    } 


    //int hexLength = stringLength(hex);
    long long hexToDec = hexToNum(hex);    


    convertNumToBin(binStorage, hexToDec);

    //int blockOffsetStartIndex = 0;
    //int blockOffsetEndIndex = blockOffsetBits - 1;
    //int blockOffset = binToDec(binStorage, blockOffsetStartIndex, blockOffsetEndIndex);


    long long setIndex;

    if (assocType == 2)
    {
        setIndex = 0;
    }
    else
    {
        int setIndexStartIndex = blockOffsetBits;
        int setIndexEndIndex = blockOffsetBits + setIndexBits - 1;
        setIndex = binToDec(binStorage, setIndexStartIndex, setIndexEndIndex);
    }

    long long tagValue = binToDec(binStorage, blockOffsetBits + setIndexBits, 48);





    // Step 3: go to the set index position in the cache.  Loop through the set
    //         (up to the numOfLines) to check if the tag matches.  If there is
    //         a match, update the cache hit counter. Update the linked list and 
    //         return the function immediately. Otherwise, update the cache miss 
    //         counter.  



    for (int i = 0; i < numOfLines; i++)
    {
        if (cache[setIndex] -> tag[i] == tagValue)
        {

            incrementHitCount();

            list[setIndex] = updateList(list[setIndex], replacePolicy, i, 0);

            return;
        }
    }



    incrementMissCount();





    // Step 4: Loop through the set (up to the numOfLines) to check if any of the valid bits
    //         equal 0 (unused).  If an unused line is found, 
    //
    //         1. update the valid bit (change it to 1)
    //         2. update the tag bits
    //         3. update the linked list
    //         4. increment readCount
    //         5. return the function



    for (int i = 0; i < numOfLines; i++)
    {
        if (cache[setIndex] -> valid[i] == 0)
        {
    
            cache[setIndex] -> valid[i] = 1;
            cache[setIndex] -> tag[i] = tagValue;

            list[setIndex] = updateList(list[setIndex], replacePolicy, i, 0);

            incrementReadCount();

            return;
        }
    }





    // Step 5: After looping through the set and there are no unused lines,
    //
    //         1. evict the correct line (last node of linked list)
    //              *MAKE SURE TO CREATE A SEPERATE LINKED LIST FOR EACH SET!!! RIGHT NOW YOU ONLY HAVE 1 FOR THE WHOLE THING!!!*
    //         2. update the linked list
    //         3. update the tag and valid bits for the first line of the set
    //         4. increment readCount
    //         5. return the function





    struct node* ptr = list[setIndex];

    while (ptr -> next != 0)
    {
        ptr = ptr -> next;
    }

    int evictLineNumber = ptr -> lineIndex;

    cache[setIndex] -> tag[evictLineNumber] = tagValue;
    cache[setIndex] -> valid[evictLineNumber] = 1;

    list[setIndex] = updateList(list[setIndex], replacePolicy, evictLineNumber, 1);

    incrementReadCount();

    return;

}



// cache write method

void reqWrite(struct set* cache[], int numOfLines, int numOfSets, int blockSize, char* hex, int assocType, int replacePolicy, struct node* list[])
{


    // Step 1: find block offset and set index bit size



    int blockOffsetBits = findBlockOffsetBits(blockSize);

    int setIndexBits = findSetIndexBits(numOfSets);

    


    // Step 2: convert the hex number into a binary form, then determine the
    //         exact values of the offset, set index, and tag given binary
    //         form.



    int binStorage[500];
    
    for (int i = 0; i < 500; i++)
    {
        binStorage[i] = 0;
    } 


    //int hexLength = stringLength(hex);
    long long hexToDec = hexToNum(hex);    


    convertNumToBin(binStorage, hexToDec);

    long long setIndex;

    if (assocType == 2)
    {
        setIndex = 0;
    }
    else
    {
        int setIndexStartIndex = blockOffsetBits;
        int setIndexEndIndex = blockOffsetBits + setIndexBits - 1;
        setIndex = binToDec(binStorage, setIndexStartIndex, setIndexEndIndex);
    }

    long long tagValue = binToDec(binStorage, blockOffsetBits + setIndexBits, 48);






    // Step 3: go to the set index position in the cache.  Loop through the set
    //         (up to the numOfLines) to check if the tag matches.  If there is
    //         a match, update the cache hit counter. Update the linked list and 
    //         return the function immediately. Otherwise, update the cache miss 
    //         counter.




    for (int i = 0; i < numOfLines; i++)
    {
        if (cache[setIndex] -> tag[i] == tagValue)
        {
            incrementHitCount();

            incrementWriteCount();

            list[setIndex] = updateList(list[setIndex], replacePolicy, i, 0);

            return;
        }
    }

    incrementMissCount();




    // Step 4: Loop through the set (up to the numOfLines) to check if any of the valid bits
    //         equal 0 (unused).  If an unused line is found, 
    //
    //         1. update the valid bit (change it to 1)
    //         2. update the tag bits
    //         3. update the linked list
    //         4. increment readCount
    //         5. return the function


    for (int i = 0; i < numOfLines; i++)
    {
        if (cache[setIndex] -> valid[i] == 0)
        {
            cache[setIndex] -> valid[i] = 1;
            cache[setIndex] -> tag[i] = tagValue;

            list[setIndex] = updateList(list[setIndex], replacePolicy, i, 0);

            incrementReadCount();

            incrementWriteCount();

            return;
        }
    }








    // Step 5: After looping through the set and there are no unused lines,
    //
    //         1. evict the correct line (last node of linked list)
    //              *MAKE SURE TO CREATE A SEPERATE LINKED LIST FOR EACH SET!!! RIGHT NOW YOU ONLY HAVE 1 FOR THE WHOLE THING!!!*
    //         2. update the linked list
    //         3. update the tag and valid bits for the first line of the set
    //         4. increment readCount
    //         5. return the function









    struct node* ptr = list[setIndex];

    while (ptr -> next != 0)
    {
        ptr = ptr -> next;
    }

    int evictLineNumber = ptr -> lineIndex;

    cache[setIndex] -> tag[evictLineNumber] = tagValue;
    cache[setIndex] -> valid[evictLineNumber] = 1;


    list[setIndex] = updateList(list[setIndex], replacePolicy, evictLineNumber, 1);


    incrementReadCount();
    incrementWriteCount();


    return;
}




// functions to check validity

int checkInvalid(int cacheSize, int blockSize)
{
    if (cacheSize <= 0)
    {
        return 1;
    }
    if (blockSize <= 0)
    {
        return 1;
    }


    int csCounter = 0;
    int lmao = 5;
    while (lmao > 4)
    {
        if ((long long)(pow(2, csCounter)) == cacheSize)
        {
            break;
        }
        if ((long long)(pow(2, csCounter))> cacheSize)
        {
            return 1;
        }

        csCounter++;
    }


    int bsCounter = 0;

    while(lmao > 4)
    {
        if ((long long)(pow(2, bsCounter)) == blockSize)
        {
            break;
        }
        if ((long long)(pow(2, bsCounter)) > blockSize)
        {
            return 1;
        }

        bsCounter++;
    }



    return 0;
}





int main (int argc, char* argv[argc + 1])
{


                                    // checks invalidity of arguments

    if (argc != 6)
    {
        printf("error\n");
        return EXIT_SUCCESS;
    }
    int convArg1 = atoi(argv[1]);
    int convArg4 = atoi(argv[4]);
    int csbsCheck = checkInvalid(convArg1, convArg4);
    if (csbsCheck == 1)
    {
        printf("error\n");
        return EXIT_SUCCESS;
    }
    if (access(argv[5], F_OK) == -1)
    {
        printf("error\n");
        return EXIT_SUCCESS;
    }
    


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



                                    // Put the command line arguments in variables

    int cacheSize = atoi(argv[1]);
    int assocType;
    int n_way;

    // Assoctype key:
    // 
    // Direct = 1                                                  
    // Fully Associative = 2
    // N-way Associative = 3, the number of ways are defined in 'n_way'

    if (strcmp(argv[2], "direct") == 0)
    {
        assocType = 1;
        n_way = -1;
    }
    else if (strcmp(argv[2], "assoc") == 0)
    {
        assocType = 2;
        n_way = -1;
    }
    else
    {
        assocType = 3;
        int counter = 0;
        char *string = argv[2];
        for (int i = 6; string[i] != '\0'; i++)
        {
            counter++;
        }
        char* dest = malloc(counter + 1);
        memcpy(dest, &string[6], counter);
        dest[counter] = '\0';
        n_way = atoi(dest);
        free(dest);
    }

    // cachePolicy key:
    //
    // FIFO = 1
    // LRU = 2

    int cachePolicy;
    if (strcmp(argv[3], "fifo") == 0)
    {
        cachePolicy = 1; 
    }
    else
    {
        cachePolicy = 2;
    }

    // Block Size:

    int blockSize = atoi(argv[4]);

    // Trace File

    FILE *fp = fopen(argv[5], "r");
    
    
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    

    // Based on the inputs given, create a cache struct

    struct set* cache[2048];
    int numOfSets;
    int numOfLines;

    if (assocType == 1) // direct mapping
    {
        numOfSets = cacheSize / blockSize;
        numOfLines = 1;
    }
    else if (assocType == 2) // fully associative
    {
        numOfSets = 1;
        numOfLines = cacheSize / blockSize;
    }
    else // n associative
    {
        numOfSets = cacheSize / (blockSize * n_way);
        numOfLines = n_way;
    }


    // allocating the cache using the given information

    for (int i = 0; i < numOfSets; i++)
    {
        cache[i] = setAllocate();
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    // Scan through the text file and update the amound of memory read/write count and cache hit/miss count

    char request;
    char hex[500];

    struct node* list[2048];
    for (int i = 0; i < 2048; i++)
    {
        list[i] = 0;
    }


    while (fscanf(fp, "%c %s\n", &request, hex) != EOF)
    {

        if (request == 'R')
        {
            reqRead(cache, numOfLines, numOfSets, blockSize, hex, assocType, cachePolicy, list);
        }
        else
        {
            reqWrite(cache, numOfLines, numOfSets, blockSize, hex, assocType, cachePolicy, list);
        }

    }
    

    printf("memread:%llu\n", readCount);
    printf("memwrite:%llu\n", writeCount);
    printf("cachehit:%llu\n", hitCount);
    printf("cachemiss:%llu\n", missCount);
    



    for (int i = 0; i < 2048; i++)
    {
        if (list[i] == 0)
        {
            continue;
        }

        struct node* ptr = list[i];
        struct node* prev = ptr;

        while (ptr != 0)
        {
            prev = ptr;
            ptr = ptr -> next;
            free(prev);
        }
    }



    for (int i = 0; i < numOfSets; i++)
    {
        free(cache[i]);
    }


    fclose(fp);
    return EXIT_SUCCESS;
}