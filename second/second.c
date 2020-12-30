#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

unsigned long long readCount = 0;
unsigned long long writeCount = 0;
unsigned long long l1hitCount = 0;
unsigned long long l1missCount = 0;
unsigned long long l2hitCount = 0;
unsigned long long l2missCount = 0;

void incrementReadCount()
{
    readCount++;
}
void incrementWriteCount()
{
    writeCount++;
}
void incrementl1HitCount()
{
    l1hitCount++;
}
void incrementl1MissCount()
{
    l1missCount++;
}
void incrementl2HitCount()
{
    l2hitCount++;
}
void incrementl2MissCount()
{
    l2missCount++;
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


//set struct and allocate

struct set
{
    long long tag[512];  
    int valid[512];
    char address[512][50];
};

struct set* setAllocate()
{
    struct set* s = malloc(sizeof(struct set));

    for (int i = 0; i < 512; i++)
    {
        s -> tag[i] = -1;
        s -> valid[i] = 0;
        strcpy(s -> address[i], "z");
    }

    return s;

}



// functions to check validity

int checkInvalid(int cacheSize, int blockSize, int cacheSize2)
{
    if (cacheSize <= 0)
    {
        return 1;
    }
    if (blockSize <= 0)
    {
        return 1;
    }
    if (cacheSize2 <= 0)
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
        if ((long long)(pow(2, csCounter)) > cacheSize)
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


    int cs2Counter = 0;

    while (lmao > 4)
    {
        if ((long long)(pow(2, cs2Counter)) == cacheSize2)
        {
            break;
        }
        if ((long long)(pow(2, cs2Counter)) > cacheSize2)
        {
            return 1;
        }
        cs2Counter++;
    }



    return 0;
}




// functions for read and write methods



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


void reqRead(struct set* cache1[], int numOfLines1, int numOfSets1, int blockSize, char* hex, int assocType1, int replacePolicy1, struct node* list1[], struct set* cache2[],
int numOfLines2, int numOfSets2, int assocType2, int replacePolicy2, struct node* list2[])
{

    // Step 1: find block offset and set index bit size

    int blockOffsetBits1 = findBlockOffsetBits(blockSize);

    int setIndexBits1 = findSetIndexBits(numOfSets1);

    


    // Step 2: convert the hex number into a binary form, then determine the
    //         exact values of the offset, set index, and tag given binary
    //         form.


    int binStorage[500];
    
    for (int i = 0; i < 500; i++)
    {
        binStorage[i] = 0;
    } 


    //int hexLength1 = stringLength(hex);
    long long hexToDec1 = hexToNum(hex);    


    convertNumToBin(binStorage, hexToDec1);

    //int blockOffsetStartIndex = 0;
    //int blockOffsetEndIndex = blockOffsetBits - 1;
    //int blockOffset = binToDec(binStorage, blockOffsetStartIndex, blockOffsetEndIndex);


    long long setIndex1;

    if (assocType1 == 2)
    {
        setIndex1 = 0;
    }
    else
    {
        int setIndexStartIndex = blockOffsetBits1;
        int setIndexEndIndex = blockOffsetBits1 + setIndexBits1 - 1;
        setIndex1 = binToDec(binStorage, setIndexStartIndex, setIndexEndIndex);
    }

    long long tagValue1 = binToDec(binStorage, blockOffsetBits1 + setIndexBits1, 48);





    // Step 3: go to the set index position in the cache.  Loop through the set
    //         (up to the numOfLines) to check if the tag matches.  If there is
    //         a match, update the cache hit counter. Update the linked list and 
    //         return the function immediately. Otherwise, update the cache miss 
    //         counter.  



    for (int i = 0; i < numOfLines1; i++)
    {
        if (cache1[setIndex1] -> tag[i] == tagValue1)
        {

            incrementl1HitCount();

            list1[setIndex1] = updateList(list1[setIndex1], replacePolicy1, i, 0);

            return;
        }
    }

    incrementl1MissCount();



    // Step 4: Recalculate the set index and the tag for L2 cache. Loop through the correct
    //         set index in L2 to see if any of the tags match
    //         
    //         If there is a tag match:
    //         
    //         1. Reset the valid, tag and address of the line (valid = 0, tag = -1, strcpy(address, "z")).
    //         2. Increment L2 hit count.
    //         3. Loop through the setIndex (cache1) to find an open line.  If an open line is found, plug in 
    //            the tag and change the valid to 1.  Plug in the address.  Update the list. Return the function.
    //         4. If no open lines are available in the set, evict the correct line (last node of the linked list).
    //            Update the valid, tag and address of that line.  Update list1 for eviction.
    //         5. Recalculate the set index and the tag for the evicted line.  Loop through the correct set in L2
    //            to see if there are any open lines.  If there is an open line, plug in the tag and address and
    //            change the valid to 1. Update list2 and return.
    //         6. If there are no open lines, evict the correct line (last node of the list).  Replace the valid, tag
    //            and address. Update L2 for eviction and return.
    //
    //         If there is not tag match, the address does not exist in either L1 or L2.  Increment L2 miss count  




    int blockOffsetBits2 = findBlockOffsetBits(blockSize);

    int setIndexBits2 = findSetIndexBits(numOfSets2); 

    long long setIndex2;    // new set index for L2 cache

    if (assocType2 == 2)
    {
        setIndex2 = 0;
    }
    else
    {
        int setIndexStartIndex = blockOffsetBits2;
        int setIndexEndIndex = blockOffsetBits2 + setIndexBits2 - 1;
        setIndex2 = binToDec(binStorage, setIndexStartIndex, setIndexEndIndex);
    }

    long long tagValue2 = binToDec(binStorage, blockOffsetBits2 + setIndexBits2, 48);       // new tag value for L2 cache



    // Looping through cache2 at setIndex2 to see if any of the tags match tagValue2

    for (int j = 0; j < numOfLines2; j++)
    {
        if (cache2[setIndex2] -> tag[j] == tagValue2)   // tag match found in cache2, reset values
        {
            cache2[setIndex2] -> valid[j] = 0;
            cache2[setIndex2] -> tag[j] = -1;
            strcpy(cache2[setIndex2] -> address[j], "z");
            incrementl2HitCount();


            // loop through cache1 to find an open valid spot

            for (int i = 0; i < numOfLines1; i++)
            {
                if (cache1[setIndex1] -> valid[i] == 0)  // open line found in set
                {
                    cache1[setIndex1] -> valid[i] = 1;
                    cache1[setIndex1] -> tag[i] = tagValue1;
                    strcpy(cache1[setIndex1] -> address[i], hex);

                    list1[setIndex1] = updateList(list1[setIndex1], replacePolicy1, i, 0);

                    return;
                }
            }

            // no open lines, something must be evicted

            struct node* ptr = list1[setIndex1];
            while (ptr -> next != 0)
            {
                ptr = ptr -> next;
            }

            int evictLineNumber1 = ptr -> lineIndex;

            char evictedHex[50];
            strcpy(evictedHex, cache1[setIndex1] -> address[evictLineNumber1]);

            cache1[setIndex1] -> tag[evictLineNumber1] = tagValue1;
            cache1[setIndex1] -> valid[evictLineNumber1] = 1;
            strcpy(cache1[setIndex1] -> address[evictLineNumber1], hex);
            list1[setIndex1] = updateList(list1[setIndex1], replacePolicy1, evictLineNumber1, 1);


            // given the evictedHex, recalculate the setIndex and tag of evictedHex to be placed into cache2

            int evictBinStorage[500];
    
            for (int i = 0; i < 500; i++)
            {
                evictBinStorage[i] = 0;
            } 

            //int evictHexLength = stringLength(evictedHex);
            long long evictHexToDec = hexToNum(evictedHex);    
            convertNumToBin(evictBinStorage, evictHexToDec);

            if (assocType2 == 2)
            {
                setIndex2 = 0;
            }
            else
            {
                int setIndexStartIndex = blockOffsetBits2;
                int setIndexEndIndex = blockOffsetBits2 + setIndexBits2 - 1;
                setIndex2 = binToDec(evictBinStorage, setIndexStartIndex, setIndexEndIndex);
            }

            tagValue2 = binToDec(evictBinStorage, blockOffsetBits2 + setIndexBits2, 48);


            // loop through the set in cache2 to find an open line

            for (int i = 0; i < numOfLines2; i++)
            {
                if (cache2[setIndex2] -> valid[i] == 0)     // open line found
                {
                    cache2[setIndex2] -> valid[i] = 1;
                    cache2[setIndex2] -> tag[i] = tagValue2;
                    strcpy(cache2[setIndex2] -> address[i], evictedHex);

                    list2[setIndex2] = updateList(list2[setIndex2], replacePolicy2, i, 0);

                    return;
                }
            }


            // no open line in set index of cache2. must evict something.

            ptr = list2[setIndex2];

            while (ptr -> next != 0)
            {
                ptr = ptr -> next;
            }

            int evictedLineNumber2 = ptr -> lineIndex;

            cache2[setIndex2] -> tag[evictedLineNumber2] = tagValue2;
            cache2[setIndex2] -> valid[evictedLineNumber2] = 1;
            strcpy(cache2[setIndex2] -> address[evictedLineNumber2], evictedHex);
            list2[setIndex2] = updateList(list2[setIndex2], replacePolicy2, evictedLineNumber2, 1);

            return;
        }



       
    }

    
    // no tag match in either L1 or L2. Increment L2 miss count

    incrementl2MissCount();





    // Step 5: Address does not exist in either L1 or L2. Loop through the set of cache1 (up to the numOfLines) to 
    //         check if any of the valid bits equal 0 (unused).  If an unused line is found, 
    //
    //         1. update the valid bit (change it to 1)
    //         2. update the tag bits
    //         3. update the linked list
    //         4. increment readCount
    //         5. return the function



    for (int i = 0; i < numOfLines1; i++)
    {
        if (cache1[setIndex1] -> valid[i] == 0) // unused line is found
        {
            cache1[setIndex1] -> valid[i] = 1;
            cache1[setIndex1] -> tag[i] = tagValue1;
            strcpy(cache1[setIndex1] -> address[i], hex);

            list1[setIndex1] = updateList(list1[setIndex1], replacePolicy1, i, 0);

            incrementReadCount();

            return;
        }
    }




    // Step 6: After looping through the set and there are no unused lines,
    //
    //         1. evict the correct line (last node of linked list). Recalculate the set index and tag for L2.
    //         2. update list1 for eviction
    //         3. update the tag, valid and address of the line in L1
    //         4. increment readCount
    //         5. Loop through the set in L2 to find any open lines.  If there is an open line, plug in valid, tag
    //            and address for that line. Update list2 and return.
    //         6. If there are no open lines, evict the correct line (last node of list2). Update the tag, valid and
    //            address of that line. Update list2 for eviction and return.





    struct node* ptr = list1[setIndex1];

    while (ptr -> next != 0)
    {
        ptr = ptr -> next;
    }

    int evictLineNumber1 = ptr -> lineIndex;


    char evictedHex[50];
    strcpy(evictedHex, cache1[setIndex1] -> address[evictLineNumber1]);


    cache1[setIndex1] -> tag[evictLineNumber1] = tagValue1;
    cache1[setIndex1] -> valid[evictLineNumber1] = 1;
    strcpy(cache1[setIndex1] -> address[evictLineNumber1], hex);

    list1[setIndex1] = updateList(list1[setIndex1], replacePolicy1, evictLineNumber1, 1);

    incrementReadCount();


    // recalculate evictedHex for cache2


    blockOffsetBits2 = findBlockOffsetBits(blockSize);

    setIndexBits2 = findSetIndexBits(numOfSets2); 

    int binStorage2[500];

    for (int i = 0; i < 500; i++)
    {
        binStorage2[i] = 0;
    }

    //int hexLength2 = stringLength(evictedHex);
    long long hexToDec2 = hexToNum(evictedHex);    

    convertNumToBin(binStorage2, hexToDec2);

    if (assocType2 == 2)
    {
        setIndex2 = 0;
    }
    else
    {
        int setIndexStartIndex = blockOffsetBits2;
        int setIndexEndIndex = blockOffsetBits2 + setIndexBits2 - 1;
        setIndex2 = binToDec(binStorage2, setIndexStartIndex, setIndexEndIndex);
    }

    tagValue2 = binToDec(binStorage2, blockOffsetBits2 + setIndexBits2, 48);


    // looking through cache2 to find an open line

    for (int i = 0; i < numOfLines2; i++)
    {
        if (cache2[setIndex2] -> valid[i] == 0)
        {
            cache2[setIndex2] -> valid[i] = 1;
            cache2[setIndex2] -> tag[i] = tagValue2;
            strcpy(cache2[setIndex2] -> address[i], evictedHex);
            list2[setIndex2] = updateList(list2[setIndex2], replacePolicy2, i, 0);
            return;
        }
    }

    // no open lines found, must evict

    ptr = list2[setIndex2];

    while (ptr -> next != 0)
    {
        ptr = ptr -> next;
    }

    int evictedLineNumber2 = ptr -> lineIndex;

    cache2[setIndex2] -> valid[evictedLineNumber2] = 1;
    cache2[setIndex2] -> tag[evictedLineNumber2] = tagValue2;
    strcpy(cache2[setIndex2] -> address[evictedLineNumber2], evictedHex);

    list2[setIndex2] = updateList(list2[setIndex2], replacePolicy2, evictedLineNumber2, 1);

    return;

}





void reqWrite(struct set* cache1[], int numOfLines1, int numOfSets1, int blockSize, char* hex, int assocType1, int replacePolicy1, struct node* list1[], struct set* cache2[],
int numOfLines2, int numOfSets2, int assocType2, int replacePolicy2, struct node* list2[])
{
    // Step 1: find block offset and set index bit size

    int blockOffsetBits1 = findBlockOffsetBits(blockSize);

    int setIndexBits1 = findSetIndexBits(numOfSets1);

    incrementWriteCount();


    // Step 2: convert the hex number into a binary form, then determine the
    //         exact values of the offset, set index, and tag given binary
    //         form.


    int binStorage[500];
    
    for (int i = 0; i < 500; i++)
    {
        binStorage[i] = 0;
    } 


    //int hexLength1 = stringLength(hex);
    long long hexToDec1 = hexToNum(hex);    


    convertNumToBin(binStorage, hexToDec1);

    //int blockOffsetStartIndex = 0;
    //int blockOffsetEndIndex = blockOffsetBits - 1;
    //int blockOffset = binToDec(binStorage, blockOffsetStartIndex, blockOffsetEndIndex);


    long long setIndex1;

    if (assocType1 == 2)
    {
        setIndex1 = 0;
    }
    else
    {
        int setIndexStartIndex = blockOffsetBits1;
        int setIndexEndIndex = blockOffsetBits1 + setIndexBits1 - 1;
        setIndex1 = binToDec(binStorage, setIndexStartIndex, setIndexEndIndex);
    }

    long long tagValue1 = binToDec(binStorage, blockOffsetBits1 + setIndexBits1, 48);





    // Step 3: go to the set index position in the cache.  Loop through the set
    //         (up to the numOfLines) to check if the tag matches.  If there is
    //         a match, update the cache hit counter. Update the linked list and 
    //         return the function immediately. Otherwise, update the cache miss 
    //         counter.  



    for (int i = 0; i < numOfLines1; i++)
    {
        if (cache1[setIndex1] -> tag[i] == tagValue1)
        {

            incrementl1HitCount();

            list1[setIndex1] = updateList(list1[setIndex1], replacePolicy1, i, 0);

            return;
        }
    }

    incrementl1MissCount();



    // Step 4: Recalculate the set index and the tag for L2 cache. Loop through the correct
    //         set index in L2 to see if any of the tags match
    //         
    //         If there is a tag match:
    //         
    //         1. Reset the valid, tag and address of the line (valid = 0, tag = -1, strcpy(address, "z")).
    //         2. Increment L2 hit count.
    //         3. Loop through the setIndex (cache1) to find an open line.  If an open line is found, plug in 
    //            the tag and change the valid to 1.  Plug in the address.  Update the list. Return the function.
    //         4. If no open lines are available in the set, evict the correct line (last node of the linked list).
    //            Update the valid, tag and address of that line.  Update list1 for eviction.
    //         5. Recalculate the set index and the tag for the evicted line.  Loop through the correct set in L2
    //            to see if there are any open lines.  If there is an open line, plug in the tag and address and
    //            change the valid to 1. Update list2 and return.
    //         6. If there are no open lines, evict the correct line (last node of the list).  Replace the valid, tag
    //            and address. Update L2 for eviction and return.
    //
    //         If there is not tag match, the address does not exist in either L1 or L2.  Increment L2 miss count  




    int blockOffsetBits2 = findBlockOffsetBits(blockSize);

    int setIndexBits2 = findSetIndexBits(numOfSets2); 

    long long setIndex2;    // new set index for L2 cache

    if (assocType2 == 2)
    {
        setIndex2 = 0;
    }
    else
    {
        int setIndexStartIndex = blockOffsetBits2;
        int setIndexEndIndex = blockOffsetBits2 + setIndexBits2 - 1;
        setIndex2 = binToDec(binStorage, setIndexStartIndex, setIndexEndIndex);
    }

    long long tagValue2 = binToDec(binStorage, blockOffsetBits2 + setIndexBits2, 48);       // new tag value for L2 cache



    // Looping through cache2 at setIndex2 to see if any of the tags match tagValue2

    for (int j = 0; j < numOfLines2; j++)
    {
        if (cache2[setIndex2] -> tag[j] == tagValue2)   // tag match found in cache2, reset values
        {
            cache2[setIndex2] -> valid[j] = 0;
            cache2[setIndex2] -> tag[j] = -1;
            strcpy(cache2[setIndex2] -> address[j], "z");
            incrementl2HitCount();

            // loop through cache1 to find an open valid spot

            for (int i = 0; i < numOfLines1; i++)
            {
                if (cache1[setIndex1] -> valid[i] == 0)  // open line found in set
                {
                    cache1[setIndex1] -> valid[i] = 1;
                    cache1[setIndex1] -> tag[i] = tagValue1;
                    strcpy(cache1[setIndex1] -> address[i], hex);

                    list1[setIndex1] = updateList(list1[setIndex1], replacePolicy1, i, 0);

                    return;
                }
            }

            // no open lines, something must be evicted

            struct node* ptr = list1[setIndex1];
            while (ptr -> next != 0)
            {
                ptr = ptr -> next;
            }

            int evictLineNumber1 = ptr -> lineIndex;

            char evictedHex[50];
            strcpy(evictedHex, cache1[setIndex1] -> address[evictLineNumber1]);

            cache1[setIndex1] -> tag[evictLineNumber1] = tagValue1;
            cache1[setIndex1] -> valid[evictLineNumber1] = 1;
            strcpy(cache1[setIndex1] -> address[evictLineNumber1], hex);
            list1[setIndex1] = updateList(list1[setIndex1], replacePolicy1, evictLineNumber1, 1);


            // given the evictedHex, recalculate the setIndex and tag of evictedHex to be placed into cache2

            int evictBinStorage[500];
    
            for (int i = 0; i < 500; i++)
            {
                evictBinStorage[i] = 0;
            } 

            //int evictHexLength = stringLength(evictedHex);
            long long evictHexToDec = hexToNum(evictedHex);    
            convertNumToBin(evictBinStorage, evictHexToDec);

            if (assocType2 == 2)
            {
                setIndex2 = 0;
            }
            else
            {
                int setIndexStartIndex = blockOffsetBits2;
                int setIndexEndIndex = blockOffsetBits2 + setIndexBits2 - 1;
                setIndex2 = binToDec(evictBinStorage, setIndexStartIndex, setIndexEndIndex);
            }

            tagValue2 = binToDec(evictBinStorage, blockOffsetBits2 + setIndexBits2, 48);


            // loop through the set in cache2 to find an open line

            for (int i = 0; i < numOfLines2; i++)
            {
                if (cache2[setIndex2] -> valid[i] == 0)     // open line found
                {
                    cache2[setIndex2] -> valid[i] = 1;
                    cache2[setIndex2] -> tag[i] = tagValue2;
                    strcpy(cache2[setIndex2] -> address[i], evictedHex);

                    list2[setIndex2] = updateList(list2[setIndex2], replacePolicy2, i, 0);

                    return;
                }
            }


            // no open line in set index of cache2. must evict something.

            ptr = list2[setIndex2];

            while (ptr -> next != 0)
            {
                ptr = ptr -> next;
            }

            int evictedLineNumber2 = ptr -> lineIndex;

            cache2[setIndex2] -> tag[evictedLineNumber2] = tagValue2;
            cache2[setIndex2] -> valid[evictedLineNumber2] = 1;
            strcpy(cache2[setIndex2] -> address[evictedLineNumber2], evictedHex);
            list2[setIndex2] = updateList(list2[setIndex2], replacePolicy2, evictedLineNumber2, 1);

            return;
        }



       
    }

    
    // no tag match in either L1 or L2. Increment L2 miss count

    incrementl2MissCount();





    // Step 5: Address does not exist in either L1 or L2. Loop through the set of cache1 (up to the numOfLines) to 
    //         check if any of the valid bits equal 0 (unused).  If an unused line is found, 
    //
    //         1. update the valid bit (change it to 1)
    //         2. update the tag bits
    //         3. update the linked list
    //         4. increment readCount
    //         5. return the function



    for (int i = 0; i < numOfLines1; i++)
    {
        if (cache1[setIndex1] -> valid[i] == 0) // unused line is found
        {
            cache1[setIndex1] -> valid[i] = 1;
            cache1[setIndex1] -> tag[i] = tagValue1;
            strcpy(cache1[setIndex1] -> address[i], hex);

            list1[setIndex1] = updateList(list1[setIndex1], replacePolicy1, i, 0);

            incrementReadCount();

            return;
        }
    }




    // Step 6: After looping through the set and there are no unused lines,
    //
    //         1. evict the correct line (last node of linked list). Recalculate the set index and tag for L2.
    //         2. update list1 for eviction
    //         3. update the tag, valid and address of the line in L1
    //         4. increment readCount
    //         5. Loop through the set in L2 to find any open lines.  If there is an open line, plug in valid, tag
    //            and address for that line. Update list2 and return.
    //         6. If there are no open lines, evict the correct line (last node of list2). Update the tag, valid and
    //            address of that line. Update list2 for eviction and return.





    struct node* ptr = list1[setIndex1];

    while (ptr -> next != 0)
    {
        ptr = ptr -> next;
    }

    int evictLineNumber1 = ptr -> lineIndex;


    char evictedHex[50];
    strcpy(evictedHex, cache1[setIndex1] -> address[evictLineNumber1]);


    cache1[setIndex1] -> tag[evictLineNumber1] = tagValue1;
    cache1[setIndex1] -> valid[evictLineNumber1] = 1;
    strcpy(cache1[setIndex1] -> address[evictLineNumber1], hex);

    list1[setIndex1] = updateList(list1[setIndex1], replacePolicy1, evictLineNumber1, 1);

    incrementReadCount();


    // recalculate evictedHex for cache2


    blockOffsetBits2 = findBlockOffsetBits(blockSize);

    setIndexBits2 = findSetIndexBits(numOfSets2); 

    int binStorage2[500];

    for (int i = 0; i < 500; i++)
    {
        binStorage2[i] = 0;
    }

    //int hexLength2 = stringLength(evictedHex);
    long long hexToDec2 = hexToNum(evictedHex);    

    convertNumToBin(binStorage2, hexToDec2);

    if (assocType2 == 2)
    {
        setIndex2 = 0;
    }
    else
    {
        int setIndexStartIndex = blockOffsetBits2;
        int setIndexEndIndex = blockOffsetBits2 + setIndexBits2 - 1;
        setIndex2 = binToDec(binStorage2, setIndexStartIndex, setIndexEndIndex);
    }

    tagValue2 = binToDec(binStorage2, blockOffsetBits2 + setIndexBits2, 48);


    // looking through cache2 to find an open line

    for (int i = 0; i < numOfLines2; i++)
    {
        if (cache2[setIndex2] -> valid[i] == 0)
        {
            cache2[setIndex2] -> valid[i] = 1;
            cache2[setIndex2] -> tag[i] = tagValue2;
            strcpy(cache2[setIndex2] -> address[i], evictedHex);
            list2[setIndex2] = updateList(list2[setIndex2], replacePolicy2, i, 0);
            return;
        }
    }

    // no open lines found, must evict

    ptr = list2[setIndex2];

    while (ptr -> next != 0)
    {
        ptr = ptr -> next;
    }

    int evictedLineNumber2 = ptr -> lineIndex;

    cache2[setIndex2] -> valid[evictedLineNumber2] = 1;
    cache2[setIndex2] -> tag[evictedLineNumber2] = tagValue2;
    strcpy(cache2[setIndex2] -> address[evictedLineNumber2], evictedHex);

    list2[setIndex2] = updateList(list2[setIndex2], replacePolicy2, evictedLineNumber2, 1);

    return;
}





int main (int argc, char* argv[argc + 1])
{

                                    // checks invalidity of arguments

    if (argc != 9)
    {
        printf("error\n");
        return EXIT_SUCCESS;
    }
    int convArg1 = atoi(argv[1]);
    int convArg4 = atoi(argv[4]);
    int convArg5 = atoi(argv[5]);
    int csbsCheck = checkInvalid(convArg1, convArg4, convArg5);
    if (csbsCheck == 1)
    {
        printf("error\n");
        return EXIT_SUCCESS;
    }
    if (access(argv[8], F_OK) == -1)
    {
        printf("error\n");
        return EXIT_SUCCESS;
    }
    


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


                                    // Put the command line arguments in variables

    int cacheSize1 = atoi(argv[1]);
    int cacheSize2 = atoi(argv[5]);
    int assocType1;
    int assocType2;
    int n_way1;
    int n_way2;

    // Assoctype key:
    // 
    // Direct = 1                                                  
    // Fully Associative = 2
    // N-way Associative = 3, the number of ways are defined in 'n_way'

    if (strcmp(argv[2], "direct") == 0)
    {
        assocType1 = 1;
        n_way1 = -1;
    }
    else if (strcmp(argv[2], "assoc") == 0)
    {
        assocType1 = 2;
        n_way1 = -1;
    }
    else
    {
        assocType1 = 3;
        int counter = 0;
        char *string = argv[2];
        for (int i = 6; string[i] != '\0'; i++)
        {
            counter++;
        }
        char* dest = malloc(counter + 1);
        memcpy(dest, &string[6], counter);
        dest[counter] = '\0';
        n_way1 = atoi(dest);
        free(dest);
    }

    if (strcmp(argv[6], "direct") == 0)
    {
        assocType2 = 1;
        n_way2 = -1;
    }
    else if (strcmp(argv[6], "assoc") == 0)
    {
        assocType2 = 2;
        n_way2 = -1;
    }
    else
    {
        assocType2 = 3;
        int counter = 0;
        char *string = argv[6];
        for (int i = 6; string[i] != '\0'; i++)
        {
            counter++;
        }
        char* dest = malloc(counter + 1);
        memcpy(dest, &string[6], counter);
        dest[counter] = '\0';
        n_way2 = atoi(dest);
        free(dest);
    }

    // cachePolicy key:
    //
    // FIFO = 1
    // LRU = 2

    int cachePolicy1;
    int cachePolicy2;
    if (strcmp(argv[3], "fifo") == 0)
    {
        cachePolicy1 = 1; 
    }
    else
    {
        cachePolicy1 = 2;
    }

    if (strcmp(argv[7], "fifo") == 0)
    {
        cachePolicy2 = 1;
    }
    else
    {
        cachePolicy2 = 2;
    }

    // Block Size:

    int blockSize = atoi(argv[4]);

    // Trace File

    FILE *fp = fopen(argv[8], "r");
    
    
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    

    // Based on the inputs given, create a cache struct

    struct set* cache1[2048];
    struct set* cache2[2048];
    int numOfSets1;
    int numOfLines1;
    int numOfSets2;
    int numOfLines2;

    if (assocType1 == 1) // direct mapping
    {
        numOfSets1 = cacheSize1 / blockSize;
        numOfLines1 = 1;
    }
    else if (assocType1 == 2) // fully associative
    {
        numOfSets1 = 1;
        numOfLines1 = cacheSize1 / blockSize;
    }
    else // n associative
    {
        numOfSets1 = cacheSize1 / (blockSize * n_way1);
        numOfLines1 = n_way1;
    }

    if (assocType2 == 1) // direct mapping
    {
        numOfSets2 = cacheSize2 / blockSize;
        numOfLines2 = 1;
    }
    else if (assocType2 == 2) // fully associative
    {
        numOfSets2 = 1;
        numOfLines2 = cacheSize2 / blockSize;
    }
    else // n associative
    {
        numOfSets2 = cacheSize2 / (blockSize * n_way2);
        numOfLines2 = n_way2;
    }


    // allocating the cache using the given information

    for (int i = 0; i < numOfSets1; i++)
    {
        cache1[i] = setAllocate();
    }

    for (int i = 0; i < numOfSets2; i++)
    {
        cache2[i] = setAllocate();
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



    // Scan through the text file and update the amound of memory read/write count and cache hit/miss count

    char request;
    char hex[500];

    struct node* list1[2048];
    struct node* list2[2048];

    for (int i = 0; i < 2048; i++)
    {
        list1[i] = 0;
        list2[i] = 0;
    }




    while (fscanf(fp, "%c %s\n", &request, hex) != EOF)
    {

        if (request == 'R')
        {
            reqRead(cache1, numOfLines1, numOfSets1, blockSize, hex, assocType1, cachePolicy1, list1, cache2, numOfLines2, numOfSets2, assocType2, cachePolicy2, list2);
        }
        else
        {
            reqWrite(cache1, numOfLines1, numOfSets1, blockSize, hex, assocType1, cachePolicy1, list1, cache2, numOfLines2, numOfSets2, assocType2, cachePolicy2, list2);
        }

    }



    printf("memread:%llu\n", readCount);
    printf("memwrite:%llu\n", writeCount);
    printf("l1cachehit:%llu\n", l1hitCount);
    printf("l1cachemiss:%llu\n", l1missCount);
    printf("l2cachehit:%llu\n", l2hitCount);
    printf("l2cachemiss:%llu\n", l2missCount);







    for (int i = 0; i < 2048; i++)
    {
        if (list1[i] == 0)
        {
            continue;
        }

        struct node* ptr = list1[i];
        struct node* prev = ptr;

        while (ptr != 0)
        {
            prev = ptr;
            ptr = ptr -> next;
            free(prev);
        }
    }

    for (int i = 0; i < 2048; i++)
    {
        if (list2[i] == 0)
        {
            continue;
        }

        struct node* ptr = list2[i];
        struct node* prev = ptr;

        while (ptr != 0)
        {
            prev = ptr;
            ptr = ptr -> next;
            free(prev);
        }
    }



    for (int i = 0; i < numOfSets1; i++)
    {
        free(cache1[i]);
    }

    for (int i = 0; i < numOfSets2; i++)
    {
        free(cache2[i]);
    }


    fclose(fp);
    return EXIT_SUCCESS;


}



