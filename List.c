#include "includes.h"
#include "List.h"
#include "Time.h"

void MapDumpSizes(ListNode *Head)
{
    int i;

    for (i=0; i < MapChainCount(Head); i++)
    {
        printf("%d %d\n",i,ListSize(MapGetNthChain(Head, i)));
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

ListNode *MapCreate(int Buckets, int Flags)
{
    ListNode *Node, *SubNode;
    int i;

//clear map flags out
    Flags &= ~LIST_FLAG_MAP;

    Node=ListCreate();
    Node->Flags |= LIST_FLAG_MAP_HEAD | Flags;
    Node->ItemType=Buckets;
    Node->Item=calloc(Buckets+1, sizeof(ListNode));
    SubNode=(ListNode *) Node->Item;
    for (i=0; i < Buckets; i++)
    {
        SubNode->Head=SubNode;
        SubNode->Prev=SubNode;
        SubNode->Flags |= LIST_FLAG_MAP_CHAIN | Flags;
        SubNode->Stats=(ListStats *) calloc(1,sizeof(ListStats));
        SubNode++;
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

    if (Map->Flags & LIST_FLAG_MAP_HEAD)
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
        i=fnv_hash(Key, Map->ItemType);
        Node=(ListNode *) Map->Item;
        return(Node + i);
    }
    return(NULL);
}




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

//avoid a function call by not calling ListGetHead
    Head=List->Head;

    if (List->Next==NULL) Head->Prev=List; /* The head Item has its Prev as being the last item! */
    Head->Stats->Hits++;

    return(Head->Stats->Hits);
}

unsigned long ListDecrNoOfItems(ListNode *List)
{
    ListNode *Head;

//avoid a function call by not calling ListGetHead
    Head=List->Head;

    if (List->Next==NULL) Head->Prev=List->Prev; /* The head Item has its Prev as being the last item! */
    if (Head->Stats->Hits > 0) Head->Stats->Hits--;
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

//avoid a function call by not calling ListGetHead
    Head=Prev->Head;
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

    Prev=Node->Prev;
    Next=Node->Next;
    if (Prev !=NULL) Prev->Next=Next;
    if (Next !=NULL) Next->Prev=Prev;

//avoid a function call by not calling ListGetHead
    Head=Node->Head;
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
    ListDecrNoOfItems(Node);
		}

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
    ListStart->Head=ListStart;
    ListStart->Prev=ListStart;
    ListSetNoOfItems(ListStart,0);
}


void ListDestroy(ListNode *ListStart, LIST_ITEM_DESTROY_FUNC ItemDestroyer)
{
    if (! ListStart) return;
    ListClear(ListStart, ItemDestroyer);
    if (ListStart->Item) free(ListStart->Item);
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



ListNode *ListInsertTypedItem(ListNode *InsertNode, uint16_t Type, const char *Name, void *Item)
{
    ListNode *NewNode;

    if (! InsertNode) return(NULL);
    NewNode=(ListNode *) calloc(1,sizeof(ListNode));
    ListThreadNode(InsertNode, NewNode);
    NewNode->Item=Item;
    NewNode->ItemType=Type;
    if (StrLen(Name)) NewNode->Tag=CopyStr(NewNode->Tag,Name);
    if (InsertNode->Head->Flags & LIST_FLAG_STATS)
    {
        NewNode->Stats=(ListStats *) calloc(1,sizeof(ListStats));
        NewNode->Stats->Time=GetTime(TIME_CACHED);
    }

    return(NewNode);
}


ListNode *ListAddTypedItem(ListNode *ListStart, uint16_t Type, const char *Name, void *Item)
{
    ListNode *Curr;

    if (ListStart->Flags & LIST_FLAG_MAP_HEAD)
    {
        //There's no real way for MapGetChain to fail, so we update stats
        //here
        if (ListStart->Stats) ListStart->Stats->Hits++;
        ListStart=MapGetChain(ListStart, Name);
    }

    Curr=ListGetLast(ListStart);
    if (Curr==NULL) return(Curr);
    return(ListInsertTypedItem(Curr,Type,Name,Item));
}






ListNode *ListFindNamedItemInsert(ListNode *Head, const char *Name)
{
    ListNode *Prev, *Curr, *Next;
    int result=0, count=0;
    int hops=0, jumps=0, miss=0;
    unsigned long long val;

    if (! Head) return(Head);
    if (! StrLen(Name)) return(Head);

    if (Head->Flags & LIST_FLAG_MAP_HEAD) Head=MapGetChain(Head, Name);

//Dont use 'ListGetNext' internally
    Curr=Head->Next;
    if (! Curr) return(Head);

//Check last item in list
    Prev=Head->Prev;
    if (Prev && (Prev != Head) && Prev->Tag)
    {
        if (Head->Flags & LIST_FLAG_CASE) result=strcmp(Prev->Tag,Name);
        else result=strcasecmp(Prev->Tag,Name);

        if ((Head->Flags & LIST_FLAG_ORDERED) && (result < 1)) return(Prev);
    }


    Prev=Head;
    while (Curr)
    {
        Next=Curr->Next;
        if (Curr->Tag)
        {
            if (Head->Flags & LIST_FLAG_CASE) result=strcmp(Curr->Tag,Name);
            else result=strcasecmp(Curr->Tag,Name);

            if (result==0)
            {
                if (Head->Flags & LIST_FLAG_SELFORG) ListSwapItems(Curr->Prev, Curr);
                return(Curr);
            }

            if ((result > 0) && (Head->Flags & LIST_FLAG_ORDERED)) return(Prev);

            //Can only get here if it's not a match
            if (Head->Flags & LIST_FLAG_TIMEOUT)
            {
                val=ListNodeGetTime(Curr);
                if ((val > 0) && (val < GetTime(TIME_CACHED)))
                {
                    Destroy(Curr->Item);
                    ListDeleteNode(Curr);
                }
            }
        }

        hops++;
        count++;


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
    if ((! Node) || (Node==Head) || (! Node->Tag)) return(NULL);
		
		//'Root' can be a Map head, rather than a list head, so we call 'ListFindNamedItemInsert' to get the correct
		//insert chain
		Head=Node->Head;
    if ((Head->Flags & LIST_FLAG_CACHE) && Head->Side && Head->Side->Tag)
    {
        Node=Head->Side;
        if (Head->Flags & LIST_FLAG_CASE) result=strcmp(Node->Tag,Name);
        else result=strcasecmp(Node->Tag,Name);
        if (
            (result==0) &&
            ( (Type==ANYTYPE) || (Type==Node->ItemType) )
        )
        {
            if (Node->Stats) Node->Stats->Hits++;
            return(Node);
        }
    }

    if (Head->Flags & LIST_FLAG_CASE) result=strcmp(Node->Tag,Name);
    else result=strcasecmp(Node->Tag,Name);

    if (
        (result==0) &&
        ( (Type==ANYTYPE) || (Type==Node->ItemType) )
    )
    {
        if (Head->Flags & LIST_FLAG_CACHE) Head->Side=Node;
        if (Node->Stats) Node->Stats->Hits++;
        return(Node);
    }
    return(NULL);
}


ListNode *ListFindNamedItem(ListNode *Head, const char *Name)
{
    return(ListFindTypedItem(Head, ANYTYPE, Name));
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
ListNode *MapGetNext(ListNode *CurrItem)
{
    ListNode *SubNode, *Head;

    if (! CurrItem) return(NULL);

    if (CurrItem->Next)
    {
        if (CurrItem->Next->Next)
        {
            //it's unlikely that we will be looking up the same item again, because maps maintain seperate chains of items
            //and the likelyhood of hitting the same chain twice is low. THIS IS NOT TRUE FOR REPEATED LOOKUPS ON A LIST
            //because with a list we go through the same items over and over again whenever looking for items in the chain

            //Thus for maps we call this prefetch code, which prefetches into the L1 cache, but not into the larger, long-term
            //L2 cache. As we're unlikely to be revisiting this chain in the near future, we don't want to pollute the L2
            //cache with it

            //This is a disaster for straight forward lists though, because they have only one chain that gets revisited on
            //every search for an item

            __builtin_prefetch (CurrItem->Next->Next, 0, 0);
            if (CurrItem->Next->Next->Tag) __builtin_prefetch (CurrItem->Next->Next->Tag, 0, 0);
        }
        return(CurrItem->Next);
    }

//'Head' here points to a BUCKET HEADER. These are marked with this flag, except the last one
//so we know when we've reached the end
    if (CurrItem->Flags & LIST_FLAG_MAP_HEAD)
    {
        CurrItem=(ListNode *) CurrItem->Item;
        if (CurrItem->Next) return(CurrItem->Next);
    }

    Head=ListGetHead(CurrItem);
    while (Head->Flags & LIST_FLAG_MAP_CHAIN)
    {
        Head++;
        if (Head->Next) return(Head->Next);
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
    return(Head->Prev);
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
                if (strcmp(Prev->Tag,Curr->Tag) < 0)
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

