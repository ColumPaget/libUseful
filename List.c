#include "includes.h"
#include "List.h"
#include "Time.h"



//get the root of a list or map. For a list this will return the same
//as ListGetHead, but for a map it will return the very top level item
//instead of the head of current list
ListNode *ListGetRoot(ListNode *Node)
{
    ListNode *Head;

    Head=ListGetHead(Node);
    if (Head->Flags & LIST_FLAG_MAP_CHAIN) Head=Head->Head;

    return(Head);
}

unsigned long ListSize(ListNode *Node)
{
    ListNode *Head;

    Head=ListGetHead(Node);
    if (Head && Head->Stats) return(Head->Stats->Hits);
    return(0);
}


void MapDumpSizes(ListNode *Head)
{
    int i;
    ListNode *Chain;

    for (i=0; i < MapChainCount(Head); i++)
    {
        Chain=MapGetNthChain(Head, i);
        printf("%d %lu\n",i, ListSize(Chain));
    }
}



ListNode *ListInit(int Flags)
{
    ListNode *Node;


    Node=(ListNode *)calloc(1,sizeof(ListNode));
    Node->Head=Node;
    Node->Prev=Node;
    Node->Flags |= Flags & 0xFFFF;
//Head Node always has stats
    Node->Stats=(ListStats *) calloc(1,sizeof(ListStats));

    return(Node);
}


//A map is an array of lists (or 'chains')
//it can have up to 65534 chains
//the flag LIST_FLAG_MAP_HEAD is used to indicate the top level item
//which holds the chains as an array in it's ->Item member.
//the flag LIST_FLAG_CHAIN_HEAD is used to indicate the first item (head) of a chain
ListNode *MapCreate(int NoOfChains, int Flags)
{
    ListNode *Node, *Chains, *SubNode;
    int i;

//clear map flags out
    Flags &= ~LIST_FLAG_MAP;

    Node=ListCreate();
    Node->Flags |= LIST_FLAG_MAP_HEAD | Flags;

    if (NoOfChains > 65534) NoOfChains=65534;
    Node->ItemType=NoOfChains;

    //we allocate one more than we will use, so the last one acts as a terminator
    Chains=(ListNode *) calloc(NoOfChains+1, sizeof(ListNode));
    Node->Item=Chains;

    for (i=0; i < NoOfChains; i++)
    {
        SubNode=Chains+i;
        SubNode->ItemType=i;
        SubNode->Head=Node;
        SubNode->Prev=SubNode;
        SubNode->Flags |= LIST_FLAG_MAP_CHAIN | Flags;
        SubNode->Stats=(ListStats *) calloc(1,sizeof(ListStats));
    }

    return(Node);
}


void ListSetFlags(ListNode *List, int Flags)
{
    ListNode *Head;

    Head=ListGetHead(List);
//only the first 16bit of flags is stored. Some flags > 16 bit effect config, but
//don't need to be stored long term
    Head->Flags=Flags & 0xFFFF;
}

void ListSetDestroyer(ListNode *List, LIST_ITEM_DESTROY_FUNC Destroyer)
{
    ListNode *Head;

    Head=ListGetRoot(List);
    if (! Head->Stats) Head->Stats=(ListStats *) calloc(1,sizeof(ListStats));
    Head->Stats->Destroyer=Destroyer;
}


void ListSetMaxItems(ListNode *List, unsigned long val, LIST_ITEM_DESTROY_FUNC Destroyer)
{
    ListNode *Head;

    Head=ListGetRoot(List);
    if (! Head->Stats) Head->Stats=(ListStats *) calloc(1,sizeof(ListStats));
    Head->Stats->Max=val;
    Head->Stats->Destroyer=Destroyer;
}


void ListNodeSetHits(ListNode *Node, int val)
{
    if (! Node->Stats) Node->Stats=(ListStats *) calloc(1,sizeof(ListStats));
    Node->Stats->Hits=val;
}




int ListNodeAddHits(ListNode *Node, int val)
{
    if (! Node->Stats) Node->Stats=(ListStats *) calloc(1,sizeof(ListStats));
    Node->Stats->Hits+=val;
    return(Node->Stats->Hits);
}

void ListNodeSetTime(ListNode *Node, time_t When)
{
    if (! Node->Stats) Node->Stats=(ListStats *) calloc(1,sizeof(ListStats));
    Node->Stats->Time=When;
}





ListNode *MapGetNthChain(ListNode *Map, int n)
{
    ListNode *Node;

    while (Map && Map->Head && (! (Map->Flags & LIST_FLAG_MAP_HEAD))) Map=Map->Head;
    if (Map && (Map->Flags & LIST_FLAG_MAP_HEAD))
    {
        Node=(ListNode *) Map->Item;
        return(Node + n);
    }
    return(NULL);
}


ListNode *MapGetChain(ListNode *Map, const char *Key)
{
    unsigned int i;
    ListNode *Node;

    if (Map->Flags & LIST_FLAG_MAP_HEAD)
    {
        i=fnv_hash((unsigned const char *) Key, Map->ItemType);
        Node=(ListNode *) Map->Item;
        return(Node + i);
    }
    return(NULL);
}



/*
Number of items is stored in the 'Stats->Hits' value of the head listnode. For normal nodes this would be
a counter of how many times the node has been accessed with 'ListFindNamedItem etc,
but the head node is never directly accessed this way, so we store the count of list items in this instead
*/

void ListSetNoOfItems(ListNode *LastItem, unsigned long val)
{
    ListNode *Head;

    Head=ListGetHead(LastItem);
    if (LastItem->Next==NULL) Head->Prev=LastItem; /* The head Item has its Prev as being the last item! */

    if (Head->Stats) Head->Stats->Hits=val;
}



unsigned long ListIncrNoOfItems(ListNode *List)
{
    ListNode *Head;

    //get the head of the list, if we've been passed a map chain
    //then it's the 'Head' at this stage, otherwise get List->Head
    Head=ListGetHead(List);

    //if the Head is the head of a map chain, then we need to update
    //it's count, but we also need to update the count for the whole map
    if (Head->Flags & LIST_FLAG_MAP_CHAIN)
    {
        Head->Stats->Hits++;

        //get map head, head of whole map,  rather than chain head
        Head=Head->Head;
    }

    //okay, for plain lists, and map heads, update Hits
    Head->Stats->Hits++;

    return(Head->Stats->Hits);
}



unsigned long ListDecrNoOfItems(ListNode *List)
{
    ListNode *Head;

    //get the head of the list, if we've been passed a map chain
    //then it's the 'Head' at this stage, otherwise get List->Head
    Head=ListGetHead(List);

    if (Head->Flags & LIST_FLAG_MAP_CHAIN)
    {
        Head->Stats->Hits--;
        //get map head, rather than chain head
        Head=Head->Head;
    }

    //okay, for plain lists, and map heads, update Hits
    Head->Stats->Hits--;

    return(Head->Stats->Hits);
}


void ListThreadNode(ListNode *Prev, ListNode *Node)
{
    ListNode *Head, *Next;

//Never thread something to itself!
    if (Prev==Node) return;

    Next=Prev->Next;
    Node->Prev=Prev;
    Prev->Next=Node;
    Node->Next=Next;

    Head=ListGetHead(Prev);
    Node->Head=Head;

// Next might be NULL! If it is, then our new node is last
// item in list, so update Head node accordingly
    if (Next) Next->Prev=Node;
    else Head->Prev=Node;

    ListIncrNoOfItems(Prev);
}


void ListUnThreadNode(ListNode *Node)
{
    ListNode *Head, *Prev, *Next;

    ListDecrNoOfItems(Node);
    Prev=Node->Prev;
    Next=Node->Next;
    if (Prev !=NULL) Prev->Next=Next;
    if (Next !=NULL) Next->Prev=Prev;

    Head=ListGetHead(Node);
    if (Head)
    {
//prev node of head points to LAST item in list
        if (Head->Prev==Node)
        {
            Head->Prev=Node->Prev;
            if (Head->Prev==Head) Head->Prev=NULL;
        }

        if (Head->Side==Node) Head->Side=NULL;
        if (Head->Next==Node) Head->Next=Next;
        if (Head->Prev==Node) Head->Prev=Prev;
    }

    //make our unthreaded node a singleton
    Node->Head=NULL;
    Node->Prev=NULL;
    Node->Next=NULL;
    Node->Side=NULL;
}



void MapClear(ListNode *Map, LIST_ITEM_DESTROY_FUNC ItemDestroyer)
{
    ListNode *Node;

    if (Map->Flags & LIST_FLAG_MAP_HEAD)
    {
        for (Node=(ListNode *) Map->Item; Node->Flags & LIST_FLAG_MAP_CHAIN; Node++) ListClear(Node, ItemDestroyer);
    }
}


void ListClear(ListNode *ListStart, LIST_ITEM_DESTROY_FUNC ItemDestroyer)
{
    ListNode *Curr,*Next;

    if (! ListStart) return;
    if (ListStart->Flags & LIST_FLAG_MAP_HEAD) MapClear(ListStart, ItemDestroyer);

    Curr=ListStart->Next;
    while (Curr)
    {
        Next=Curr->Next;
        if (ItemDestroyer && Curr->Item) ItemDestroyer(Curr->Item);
        DestroyString(Curr->Tag);
        if (Curr->Stats) free(Curr->Stats);
        free(Curr);
        Curr=Next;
    }

    ListStart->Next=NULL;
    ListStart->Side=NULL;
    ListStart->Head=ListStart;
    ListStart->Prev=ListStart;
    ListSetNoOfItems(ListStart,0);
}


void ListDestroy(ListNode *ListStart, LIST_ITEM_DESTROY_FUNC ItemDestroyer)
{
    if (! ListStart) return;
    ListClear(ListStart, ItemDestroyer);
    if (ListStart->Item) free(ListStart->Item);
    if (ListStart->Stats) free(ListStart->Stats);
    free(ListStart);
}



void ListAppendItems(ListNode *Dest, ListNode *Src, LIST_ITEM_CLONE_FUNC ItemCloner)
{
    ListNode *Curr;
    void *Item;

    Curr=ListGetNext(Src);
    while (Curr !=NULL)
    {
        if (ItemCloner)
        {
            Item=ItemCloner(Curr->Item);
            ListAddNamedItem(Dest, Curr->Tag, Item);
        }
        else ListAddNamedItem(Dest, Curr->Tag, Curr->Item);
        Curr=ListGetNext(Curr);
    }
}


ListNode *ListClone(ListNode *ListStart, LIST_ITEM_CLONE_FUNC ItemCloner)
{
    ListNode *NewList;

    NewList=ListCreate();

    ListAppendItems(NewList, ListStart, ItemCloner);
    return(NewList);
}


//this function handled the 'maximum items in list' feature
static void ListHandleMaxSize(ListNode *Node, ListNode *Head)
{
    ListNode *ChainHead, *Curr;
    int i;

    //In maps ChainHead will be the head of the current subchain in the map
    //in lists it will be the same as 'Head'
    ChainHead=ListGetHead(Node);

    //we delete up to 3 items, as we're only adding 1, then even if somehow the
    //list has grown over 'Max' it will gradually shrink back
    for (i=0; (i < 3) && (ChainHead->Stats->Hits > Head->Stats->Max); i++)
    {
        Curr=ListGetNext(ChainHead);
        if (! Curr) break;

        if (Head->Stats->Destroyer) Head->Stats->Destroyer(Curr->Item);
        ListDeleteNode(Curr);
    }
}


ListNode *ListInsertTypedItem(ListNode *InsertNode, uint16_t Type, const char *Name, void *Item)
{
    ListNode *NewNode, *Head, *Curr;
    int i;

    if (! InsertNode) return(NULL);

    Head=ListGetRoot(InsertNode);

    if (Head->Stats->Max > 0) ListHandleMaxSize(InsertNode, Head);

    NewNode=(ListNode *) calloc(1,sizeof(ListNode));
    ListThreadNode(InsertNode, NewNode);
    NewNode->Item=Item;
    NewNode->ItemType=Type;
    if (StrValid(Name)) NewNode->Tag=CopyStr(NewNode->Tag,Name);

    if (Head->Flags & LIST_FLAG_STATS)
    {
        NewNode->Stats=(ListStats *) calloc(1,sizeof(ListStats));


        //If list is being used with LIST_FLAG_TIMEOUT then user will
        //call 'ListSetTime' after inserting the item, and overwrite
        //this time val, so it does no harm to set it here.
        NewNode->Stats->Time=GetTime(TIME_CACHED);
    }

    return(NewNode);
}


ListNode *ListAddTypedItem(ListNode *ListStart, uint16_t Type, const char *Name, void *Item)
{
    ListNode *Curr;

    if (ListStart->Flags & LIST_FLAG_MAP_HEAD) ListStart=MapGetChain(ListStart, Name);

    if (ListStart->Flags & LIST_FLAG_ORDERED) Curr=ListFindNamedItemInsert(ListStart, Name);
    else Curr=ListGetLast(ListStart);

    if (Curr==NULL) return(Curr);
    return(ListInsertTypedItem(Curr,Type,Name,Item));
}


#define LIST_FIND_LESSER  1
#define LIST_FIND_GREATER 2

static inline int ListConsiderInsertPoint(ListNode *Head, ListNode *Prev, const char *Name, int FindType)
{
    int result;

    if (Prev && (Prev != Head) && Prev->Tag)
    {
        if (Head->Flags & LIST_FLAG_CASE) result=strcmp(Prev->Tag,Name);
        else result=strcasecmp(Prev->Tag,Name);

        if (result == 0) return(TRUE);
        if (Head->Flags & LIST_FLAG_ORDERED)
        {
            if ((FindType == LIST_FIND_LESSER) && (result < 1)) return(TRUE);
            if ((FindType == LIST_FIND_GREATER) && (result > 1)) return(TRUE);
        }
    }

    return(FALSE);
}


ListNode *ListFindNamedItemInsert(ListNode *Root, const char *Name)
{
    ListNode *Prev=NULL, *Curr, *Next, *Head;
    int result=0;
    unsigned long long val;

    if (! Root) return(Root);
    if (! StrValid(Name)) return(Root);

    if (Root->Flags & LIST_FLAG_MAP_HEAD) Head=MapGetChain(Root, Name);
    else Head=Root;

    //Check last item in list, it it's a match or we're an ordered list and it's lesser,
    //then jump to it
    if (ListConsiderInsertPoint(Head, Head->Prev, Name, LIST_FIND_LESSER)) return(Head->Prev);

    Prev=Head;
    Curr=Head->Next;

    //if LIST_FLAG_CACHE is set, then the general purpose 'Side' pointer of the head node points to a cached item
    //if it's a match we can jump there, if an ordered list and it's less, we might as well jump to it too
    if ((Root->Flags & LIST_FLAG_CACHE) && Head->Side && Head->Side->Tag)
    {
        if (ListConsiderInsertPoint(Head, Head->Side, Name, LIST_FIND_LESSER))
        {
            Curr=Head->Side;
            //we will actually return Prev rather than Curr, because of how the
            //loop below works
            Prev=Curr->Prev;
        }
    }

    while (Curr)
    {
        //we get 'Next' here because we might delete 'Curr' if we are operating
        //as a list with 'timeouts' (LIST_FLAG_TIMEOUT)
        Next=Curr->Next;

        if (Curr->Tag)
        {
            //if the current item is a match, or we are in an ordered list and it's
            //greater, then insert between it and Prev
            if (ListConsiderInsertPoint(Head, Curr, Name, LIST_FIND_GREATER))
            {
                //as we are looking for an insert point, we'd normally return 'Prev'
                //but if Prev is a List Head or Chain head, return Curr instead
                if (Prev->Flags & LIST_FLAG_MAP_CHAIN) return(Curr);
                if (Prev == Curr->Head) return(Curr);
                return(Prev);
            }

            //Can only get here if it's not a match, in which
            //case we can safely delete any 'timed out' items
            if (Root->Flags & LIST_FLAG_TIMEOUT)
            {
                val=ListNodeGetTime(Curr);
                if ((val > 0) && (val < GetTime(TIME_CACHED)))
                {
                    if (Root->Stats && Root->Stats->Destroyer) Root->Stats->Destroyer(Curr->Item);
                    ListDeleteNode(Curr);
                }
            }
        }
        Prev=Curr;
        Curr=Next;
    }

    return(Prev);
}



ListNode *ListFindTypedItem(ListNode *Root, int Type, const char *Name)
{
    ListNode *Node, *Head;
    int result;

    if (! Root) return(NULL);
    Node=ListFindNamedItemInsert(Root, Name);
    if (! Node) return(NULL);
    Head=Node->Head;
    if (Node == Head) Node=Node->Next;


    //item must have a name, and can't be the 'head' of the list
    //if ((! Node) || (Node==Node->Head) || (! Node->Tag)) return(NULL);
    if ((! Node) || (! Node->Tag)) return(NULL);

    //'Root' can be a Map head, rather than a list head, so we call 'ListFindNamedItemInsert' to get the correct
    //insert chain


    if (Head)
    {
        //rewind, as it's possible that we've found a node mid way through a load of
        //nodes with the same tag, so rewind to the first before we consider them all
        while (Node)
        {
            if (Node->Prev==Head) break;
            if (! ListConsiderInsertPoint(Head, Node->Prev, Name, 0)) break;
            Node=Node->Prev;
        }

        while (Node)
        {
            if (Head->Flags & LIST_FLAG_CASE) result=CompareStr(Node->Tag,Name);
            else result=CompareStrNoCase(Node->Tag,Name);

            if (result==0)
            {
                if  ( (Type==LIST_ITEM_ANYTYPE) || (Type==Node->ItemType) )
                {
                    if (Head->Flags & LIST_FLAG_CACHE) Head->Side=Node;
                    if (Node->Stats)
                    {
                        Node->Stats->Hits++;

                        //if this list is being used with LIST_FLAG_TIMEOUT then the Time field in ListStats
                        //will be an expiry time, so we can't update it with a 'last accessed' time
                        if (! (Head->Flags & LIST_FLAG_TIMEOUT)) Node->Stats->Time=GetTime(TIME_CACHED);
                    }

                    if (
                        (Node != Head) &&
                        (Head->Flags & LIST_FLAG_SELFORG) &&
                        (! (Head->Flags & LIST_FLAG_ORDERED))
                    ) ListSwapItems(Node->Prev, Node);

                    return(Node);
                }

                //if this is set then there's at most one instance of an item with a given name
                //so if the above didn't match, we won't get a match
                if (Head->Flags & LIST_FLAG_UNIQ) break;
            }

            //if it's an ordered list and the strcmp says we've gone beyond the given name
            //in the list, then we won't get a match
            if ((Head->Flags & LIST_FLAG_ORDERED) && (result > 0)) break;


            Node=ListGetNext(Node);
        }
    }

    return(NULL);
}


ListNode *ListFindNamedItem(ListNode *Head, const char *Name)
{
    return(ListFindTypedItem(Head, LIST_ITEM_ANYTYPE, Name));
}



ListNode *InsertItemIntoSortedList(ListNode *List, void *Item, int (*LessThanFunc)(void *, void *, void *))
{
    ListNode *Curr, *Prev;

    Prev=List;
    Curr=Prev->Next;
    while (Curr && (LessThanFunc(NULL, Curr->Item,Item)) )
    {
        Prev=Curr;
        Curr=Prev->Next;
    }

    return(ListInsertItem(Prev,Item));
}

//list get next is just a macro that either calls this for maps, or returns Node->next
ListNode *MapChainGetNext(ListNode *CurrItem)
{
    if (! CurrItem) return(NULL);

    if (CurrItem->Next)
    {
        return(CurrItem->Next);
    }

    if (CurrItem->Flags & LIST_FLAG_MAP_HEAD)
    {
        CurrItem=(ListNode *) CurrItem->Item;
        if (CurrItem->Next) return(CurrItem->Next);
    }

    return(NULL);
}


ListNode *MapGetNext(ListNode *CurrItem)
{
    ListNode *SubNode, *ChainHead;

    if (! CurrItem) return(NULL);
    SubNode=MapChainGetNext(CurrItem);
    if (SubNode) return(SubNode);

    //if the CurrItem is a MAP_HEAD, the very top of a map, then ChainHead is the
    //first chain in it's collection of chains, held in '->Item'
    if (CurrItem->Flags & LIST_FLAG_MAP_HEAD) ChainHead=(ListNode *) CurrItem->Item;
    //if the CurrItem is the head of a chain, then it's the chain head
    else if (CurrItem->Flags & LIST_FLAG_MAP_CHAIN) ChainHead=CurrItem;
    //otherwise get the chain head
    else ChainHead=ListGetHead(CurrItem);

    while (ChainHead && (ChainHead->Flags & LIST_FLAG_MAP_CHAIN))
    {
        ChainHead=MapGetNthChain(ChainHead->Head, ChainHead->ItemType +1);
        if (ChainHead->Next) return(ChainHead->Next);
    }

    return(NULL);
}



ListNode *ListGetPrev(ListNode *CurrItem)
{
    ListNode *Prev;

    if (CurrItem == NULL) return(NULL);
    Prev=CurrItem->Prev;
    /* Don't return the dummy header! */
    if (Prev && (Prev->Prev !=NULL) && (Prev != Prev->Head)) return(Prev);
    return(NULL);
}


ListNode *ListGetLast(ListNode *CurrItem)
{
    ListNode *Head;

    Head=ListGetHead(CurrItem);
    if (! Head) return(CurrItem);
    /* the dummy header has a 'Prev' entry that points to the last item! */
    if (Head->Prev) return(Head->Prev);
    return(Head);
}



ListNode *ListGetNth(ListNode *Head, int n)
{
    ListNode *Curr;
    int count=0;

    if (! Head) return(NULL);

    Curr=ListGetNext(Head);
    while (Curr && (count < n))
    {
        count++;
        Curr=ListGetNext(Curr);
    }
    if (count < n) return(NULL);
    return(Curr);
}





ListNode *ListJoin(ListNode *List1, ListNode *List2)
{
    ListNode *Curr, *StartOfList2;

    Curr=List1;
    /*Lists all have a dummy header!*/
    StartOfList2=List2->Next;

    while (Curr->Next !=NULL) Curr=Curr->Next;
    Curr->Next=StartOfList2;
    StartOfList2->Prev=Curr;

    while (Curr->Next !=NULL) Curr=Curr->Next;
    return(Curr);
}


//Item1 is before Item2!
void ListSwapItems(ListNode *Item1, ListNode *Item2)
{
    ListNode *Head, *Prev, *Next;

    if (! Item1) return;
    if (! Item2) return;

//Never swap with a list head!
    Head=ListGetHead(Item1);
    if (Head==Item1) return;
    if (Head==Item2) return;

    Prev=Item1->Prev;
    Next=Item2->Next;

    if (Head->Next==Item1) Head->Next=Item2;
    if (Head->Prev==Item1) Head->Prev=Item2;

    if (Prev) Prev->Next=Item2;
    Item1->Prev=Item2;
    Item1->Next=Next;

    if (Next) Next->Prev=Item1;
    Item2->Prev=Prev;
    Item2->Next=Item1;

}


void ListSort(ListNode *List, void *Data, int (*LessThanFunc)(void *, void *, void *))
{
    ListNode *Curr=NULL, *Prev=NULL;
    int sorted=0;

    while (! sorted)
    {
        sorted=1;
        Prev=NULL;
        Curr=ListGetNext(List);
        while (Curr)
        {
            if (Prev !=NULL)
            {
                if ( (*LessThanFunc)(Data,Curr->Item,Prev->Item) )
                {
                    sorted=0;
                    ListSwapItems(Prev,Curr);
                }
            }
            Prev=Curr;
            Curr=ListGetNext(Curr);
        }
    }

}

void ListSortNamedItems(ListNode *List)
{
    ListNode *Curr=NULL, *Prev=NULL;
    int sorted=0;

    while (! sorted)
    {
        sorted=1;
        Prev=NULL;
        Curr=ListGetNext(List);
        while (Curr)
        {
            if (Prev !=NULL)
            {
                if (CompareStr(Prev->Tag,Curr->Tag) < 0)
                {
                    sorted=0;
                    ListSwapItems(Prev,Curr);
                }
            }

            Prev=Curr;
            Curr=ListGetNext(Curr);
        }
    }

}





ListNode *ListFindItem(ListNode *Head, void *Item)
{
    ListNode *Curr;

    if (! Item) return(NULL);
    Curr=ListGetNext(Head);
    while (Curr)
    {
        if (Curr->Item==Item)
        {
            if (Head->Flags & LIST_FLAG_SELFORG) ListSwapItems(Curr->Prev, Curr);
            return(Curr);
        }
        Curr=ListGetNext(Curr);
    }
    return(Curr);
}


void *ListDeleteNode(ListNode *Node)
{
    void *Contents;

    if (Node==NULL) return(NULL);

    ListUnThreadNode(Node);
    if (Node->Stats) free(Node->Stats);
    Contents=Node->Item;
    Destroy(Node->Tag);
    free(Node);
    return(Contents);
}

