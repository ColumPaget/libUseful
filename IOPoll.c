#include "IOPoll.h"


#define EPOLL_ADDED LIST_FLAG_USER1
#define SELECT_ADD_ERROR -1


typedef struct
{
    int pollfd;
    int size;
    int wsize;
    int high;
    void *items;
    void *witems;
} TSelectSet;




static void PollTimevalAdjust(uint64_t timeout, uint64_t start, struct timeval *tv)
{
    uint64_t diff;

    diff=GetTime(TIME_MILLISECS) - start;
    if (diff > 0)
    {
        //diff can actually be greater than 'timeout', likely due to
        //context switches, so handle that
        if (diff > timeout) timeout=0;
        else timeout-=diff;

        if (timeout > 0)
        {
            tv->tv_sec=(int) (timeout / 1000.0);
            tv->tv_usec=(timeout - (tv->tv_sec * 1000.0)) * 1000;
        }
        else
        {
            tv->tv_sec=0;
            tv->tv_usec=0;
        }
    }
}



#ifdef HAVE_EPOLL
#include <sys/epoll.h>


typedef struct
{
    int pollfd;
} T_POLL_CTX;


typedef struct
{
    int fd;
    int type;
    int result;
    int flags;
} T_EPOLL_ITEMS;


static TSelectSet *EPOLLInit(ListNode *Streams, TSelectSet *Set)
{
    ListNode *Head;
    T_POLL_CTX *Ctx;

    Head=ListGetHead(Streams);
    if (Head->Item) Ctx=(T_POLL_CTX *) Head->Item;
    else
    {
        Ctx=(T_POLL_CTX *) calloc(1, sizeof(T_POLL_CTX));
        Ctx->pollfd=epoll_create1(EPOLL_CLOEXEC);
        Head->Item=(void *) Ctx;
    }

    Set->pollfd=Ctx->pollfd;

    return(Set);
}

static int EPOLLAddFD(TSelectSet *Set, int type, int fd, int Flags)
{
    int pos;
    T_EPOLL_ITEMS *items;
    struct epoll_event event;
    int RetVal=Flags;

    if (! (Flags & EPOLL_ADDED))
    {
        if (type == SELECT_WRITE) event.events=EPOLLOUT;
        else event.events = EPOLLIN;

        event.data.fd = fd;

        if (epoll_ctl(Set->pollfd, EPOLL_CTL_ADD, fd, &event) != 0) RaiseError(ERRFLAG_ERRNO, "SelectWait(epoll)", "failed to add file descriptor '%d' using epoll", fd);

        RetVal=Flags | EPOLL_ADDED;
    }

    pos=Set->size;
    Set->size++;
    Set->items=realloc(Set->items, sizeof(T_EPOLL_ITEMS) * Set->size);
    items=(T_EPOLL_ITEMS *) Set->items;
    items[pos].fd=fd;
    items[pos].type=type;
    items[pos].result=0;
    items[pos].flags=Flags;


    return(RetVal);
}


static void EPOLLRemoveFD(T_POLL_CTX *Ctx, int fd)
{
    if (! Ctx) return;
    if (fd < 0) return;

    if (epoll_ctl(Ctx->pollfd, EPOLL_CTL_DEL, fd, NULL) != 0) RaiseError(ERRFLAG_ERRNO, "SelectWait(epoll)", "failed to delete file descriptor '%d' using epoll", fd);
}


static int EPOLLWait(TSelectSet *Set, struct timeval *tv)
{
    int event_count=-1;
    struct epoll_event event;
    int timeout=-1;
    uint64_t timeout64;
    uint64_t start, diff;
    int result;
    int fd, i;
    T_EPOLL_ITEMS *items;


    if (Set->pollfd > -1)
    {
        if (tv)
        {
            start=GetTime(TIME_MILLISECS);
            timeout=TimevalToMillisecs(tv, &timeout64);
        }

        items=(T_EPOLL_ITEMS *) Set->items;

        event_count = epoll_wait(Set->pollfd, &event, 1, timeout);
        if (tv) PollTimevalAdjust(timeout64, start,  tv);

        if (event_count > 0)
        {
            for (i=0; i < Set->size; i++)
            {
                if (items[i].fd==event.data.fd)
                {
                    if (event.events & EPOLLOUT) items[i].result |= SELECT_WRITE;
                    if (event.events & EPOLLIN) items[i].result |= SELECT_READ;
                    if (event.events & EPOLLHUP) items[i].result |= SELECT_READ;
                    if (event.events & EPOLLRDHUP) items[i].result |= SELECT_READ;
                    if (event.events & EPOLLERR) items[i].result |= SELECT_READ;
                    if (event.events & EPOLLPRI) items[i].result |= SELECT_READ;
                    break;
                }
            }
        }
    }
    else RaiseError(ERRFLAG_ERRNO, "SelectWait:(epoll)", "failed to add file descriptor '%d' using epoll", fd);


    return(event_count);
}


static int EPOLLCheck(TSelectSet *Set, int fd)
{
    int i, RetVal=0;
    T_EPOLL_ITEMS *items;

    items=(T_EPOLL_ITEMS *) Set->items;
    for (i=0; i < Set->size; i++)
    {
        if (items[i].fd==fd) return(items[i].result);
    }

    return(FALSE);
}

#endif




#ifdef HAVE_POLL

// Functions when using 'poll' as the polling mechanism

#include <poll.h>
#include <math.h>

static int SelectAddFD(TSelectSet *Set, int type, int fd)
{
    struct pollfd *items;

    if (fd > -1)
    {
        Set->size++;
        Set->items=realloc(Set->items, sizeof(struct pollfd) * Set->size);

        items=(struct pollfd *) Set->items;
        items[Set->size-1].fd=fd;
        items[Set->size-1].events=0;
        items[Set->size-1].revents=0;
        if (type & SELECT_READ) items[Set->size-1].events |= POLLIN;
        if (type & SELECT_WRITE) items[Set->size-1].events |= POLLOUT;
    }

    return(TRUE);
}




static int SelectWait(TSelectSet *Set, struct timeval *tv)
{
    int timeout=-1;
    uint64_t start, timeout64;
    int result;


    if (tv)
    {
        timeout=TimevalToMillisecs(tv, &timeout64);
        start=GetTime(TIME_MILLISECS);
    }

    result=poll((struct pollfd *) Set->items, Set->size, timeout);
    if (tv) PollTimevalAdjust(timeout64, start,  tv);

    return(result);
}



static int SelectCheck(TSelectSet *Set, int fd)
{
    int i, RetVal=0;
    struct pollfd *items;

    if (fd > -1)
    {
        items=(struct pollfd *) Set->items;
        for (i=0; i < Set->size; i++)
        {
            if (items[i].fd==fd)
            {
                if (items[i].revents & (POLLIN | POLLHUP)) RetVal |= SELECT_READ;
                if (items[i].revents & POLLOUT) RetVal |= SELECT_WRITE;
                break;
            }
        }
    }

    return(RetVal);
}
#else


// Functions when using 'select' as the polling mechanism

static int SelectAddFD(TSelectSet *Set, int type, int fd)
{
    int RetVal=TRUE;

    if (fd  < 0)
    {
        RaiseError(ERRFLAG_DEBUG, "SelectAddFD", "File Descriptor '%d' is < 0.", fd);
        return(FALSE);
    }


    if (type & SELECT_WRITE)
    {
        if (Set->wsize < FD_SETSIZE)
        {
            if (! Set->witems) Set->witems=calloc(1, sizeof(fd_set));
            FD_SET(fd, (fd_set *) Set->witems);
            Set->wsize++;
        }
        else
        {
            RaiseError(ERRFLAG_DEBUG, "SelectAddFD", "File Descriptor '%d' is higher than FD_SETSIZE limit. Cannot add to select.", fd);
            RetVal=FALSE;
        }
    }

    if (type & SELECT_READ)
    {
        if (Set->size < FD_SETSIZE)
        {
            if (! Set->items) Set->items=calloc(1, sizeof(fd_set));
            FD_SET(fd, (fd_set *) Set->items);
            Set->size++;
        }
        else
        {
            RaiseError(ERRFLAG_DEBUG, "SelectAddFD", "File Descriptor '%d' is higher than FD_SETSIZE limit. Cannot add to select.", fd);
            RetVal=FALSE;
        }
    }

    if (fd > Set->high) Set->high=fd;


    return(RetVal);
}


static int SelectWait(TSelectSet *Set, struct timeval *tv)
{
    return(select(Set->high+1, Set->items, Set->witems, NULL, tv));
}

static int SelectCheck(TSelectSet *Set, int fd)
{
    int RetVal=0;

    if (FD > -1)
    {
        if (Set->items  && FD_ISSET(fd, (fd_set *) Set->items )) RetVal |= SELECT_READ;
        if (Set->witems && FD_ISSET(fd, (fd_set *) Set->witems)) RetVal |= SELECT_WRITE;
    }
    else RaiseError(ERRFLAG_DEBUG, "SelectCheck", "File Descriptor '%d' is < 0.", fd);

    return(RetVal);
}

#endif


static void SelectSetDestroy(TSelectSet *Set)
{
    Destroy(Set->items);
    Destroy(Set->witems);
    Destroy(Set);
}





int FDSelect(int fd, int type, struct timeval *tv)
{
    TSelectSet *Set;
    int result, RetVal=STREAM_CLOSED;

    if (fd > -1)
    {
        Set=(TSelectSet *) calloc(1,sizeof(TSelectSet));
        SelectAddFD(Set, type, fd);
        result=SelectWait(Set, tv);

        if ((result==-1) && (errno==EBADF)) RetVal=STREAM_CLOSED;
        else if (result  > 0) RetVal=SelectCheck(Set, fd);
				else RetVal=0;

        SelectSetDestroy(Set);
    }
    else RaiseError(ERRFLAG_DEBUG, "FDSelect", "File Descriptor '%d' is < 0.", fd);

    return(RetVal);
}


int FDIsWritable(int fd)
{
    struct timeval tv;

    if (fd < 0) return(FALSE);

    tv.tv_sec=0;
    tv.tv_usec=0;
    if (FDSelect(fd, SELECT_WRITE, &tv) & SELECT_WRITE) return(TRUE);
    return(FALSE);
}



int FDCheckForBytes(int fd)
{
    struct timeval tv;

    if (fd < 0) return(FALSE);

    tv.tv_sec=0;
    tv.tv_usec=0;
    if (FDSelect(fd, SELECT_READ, &tv) & SELECT_READ) return(TRUE);

    return(FALSE);
}



/* The following functions all relate to 'STREAMSelect' which is a to STREAM objects what 'select' is to file descriptors
   The STREAM objects are passed as a libUseful List, and these lists are added to a list called 'STREAMSelectsList'.
	 This is done so that when, for instance, a STREAM object is destroyed, we can remove it from the list.

	 STREAMSelect can use linux epoll for STREAMSelect lists greater than five items, if libUseful was compiled with epoll support.
	 For lists less than five items, or if epoll is not compiled in, it falls back to poll/select.
*/


//Holds a list of STREAMSelect Lists, so we can clean them up in various ways
static ListNode *STREAMSelectsList=NULL;



//remove a STREAMSelect List from our list of lists, this is usually called
//by ListDestroy
void STREAMSelectUnregister(ListNode *Select)
{
    ListNode *Curr, *SelectList, *Head;

    Curr=ListFindItem(STREAMSelectsList, Select);
    if (Curr)
    {
        SelectList=(ListNode *) Curr->Item;
// EPOLL stores a context in Head->Item, get rid of that
        Head=ListGetHead(SelectList);
        if (Head->Item) free(Head->Item);
        Head->Item=NULL;
    }

//remove the stream select from our list
    ListDeleteNode(Curr);
}



//remove a STREAM from a STREAMSelect list. This sets the pointer to the STREAM object to NULL
//we don't remove the list entry in can the application program holds a pointr to it, we just
//make it point to NULL
static void STREAMSelectRemove(ListNode *Select, STREAM *S)
{
    ListNode *Curr, *Head;

    if (! S) return;
    Curr=ListFindItem(Select, S);
    if (Curr)
    {
        Head=ListGetHead(Select);

#ifdef HAVE_EPOLL
        EPOLLRemoveFD((T_POLL_CTX *) Head->Item, S->in_fd);
#endif

//remove STREAM from list. STREAMSelect will ignore this entry
        Curr->Item=NULL;

//Do not delete the list node! The application might have this in hand, and
//freeing it will result in accessing freed memory
        ListDeleteNode(Curr);
    }

}


//Remove a stream from our STREAMSelect lists. This is normally
//called by STREAMClose
void STREAMSelectsRemoveStream(STREAM *S)
{
    ListNode *Curr;

    if (! S) return;

    Curr=ListGetNext(STREAMSelectsList);
    while (Curr)
    {
        STREAMSelectRemove((ListNode *) Curr->Item, S);
        Curr=ListGetNext(Curr);
    }
}



//From here on in all functions are subfunctions of the STREAMSelect function

static TSelectSet *STREAMSelectInit(ListNode *Streams, int UseEPOLL)
{
    TSelectSet *Set;

    if (! STREAMSelectsList) STREAMSelectsList=ListCreate();
    if (! ListFindItem(STREAMSelectsList, Streams)) ListAddItem(STREAMSelectsList, Streams);

    Set=(TSelectSet *) calloc(1,sizeof(TSelectSet));
#ifdef HAVE_EPOLL
    if (UseEPOLL) EPOLLInit(Streams, Set);
#endif


    return(Set);
}


//Add a single STREAM to the select. We pass the List entry, rathr than the stream itself
//because we want to set some flags in the list entry
static int STREAMSelectAddStream(TSelectSet *Set, ListNode *Curr, int UseEPOLL)
{
    STREAM *S;

    S=(STREAM *) Curr->Item;
    if (S && (! (S->State & LU_SS_EMBARGOED)))
    {
        //server type streams don't have buffers
        if ( (S->Type != STREAM_TYPE_UNIX_SERVER) && (S->Type != STREAM_TYPE_TCP_SERVER) )
        {
            //Pump any data in the stream
            STREAMFlush(S);

            //if there's stuff in buffer, then we don't need to select the file descriptor.
            //return TRUE and STREAMSelect will return this stream.
            if (S->InEnd > S->InStart) return(TRUE);
        }

#ifdef HAVE_EPOLL
        if (UseEPOLL) Curr->Flags=EPOLLAddFD(Set, SELECT_READ, S->in_fd, Curr->Flags);
        else
#endif

            // if using 'select' for polling, we can encounter a situation where
            // we have more than FD_SETSIZE items in the list. In that situation
            // SelectAddFD will return FALSE for items over the FD_SETSIZE limit
            // We re-thread those items in the hope they will 'get their turn'
            // next time
            if (! SelectAddFD(Set, SELECT_READ, S->in_fd)) return(SELECT_ADD_ERROR);
    }

    return(FALSE);
}


//Go through all streams in the List, and add them to the Select
STREAM *STREAMSelectAddStreams(TSelectSet *Set, ListNode *Streams, int UseEPOLL)
{
    ListNode *Curr, *Next;
    int result;

    Curr=ListGetNext(Streams);
    while (Curr)
    {
        //as there are circumstances where we can move Curr
        //in the list, get Next before we do anything
        Next=ListGetNext(Curr);

        //if STREAMSelectAddStream returns TRUE then it means
        //a stream has data buffered, and we must return that or
        //risk the data sitting there forever, if all data has been
        //read into the stream buffer
        //if it returns SELECT_ADD_ERROR it means we couldn't add the stream
        //and so we move it to the start of the list, giving it 'more priority'
        //next time
        switch (STREAMSelectAddStream(Set, Curr, UseEPOLL))
        {
        case TRUE:
            return( (STREAM *) Curr->Item );
            break;
        case SELECT_ADD_ERROR:
            ListMoveStart(Curr);
            break;
        }

        Curr=Next;
    }

    return(NULL);
}


//do the actual 'select', wait for activity on any of the configured streams
static int STREAMSelectWait(TSelectSet *Set, struct timeval *tv, int UseEPOLL)
{
#ifdef HAVE_EPOLL
    if (UseEPOLL) return(EPOLLWait(Set, tv));
#endif

    return(SelectWait(Set, tv));
}



//Check Which of the STREAMs has activity
static int STREAMSelectCheck(TSelectSet *Set, STREAM *S, int UseEPOLL)
{
    if (! S) return(FALSE);

#ifdef HAVE_EPOLL
    if (UseEPOLL) return(EPOLLCheck(Set, S->in_fd));
#endif

    return(SelectCheck(Set, S->in_fd));
}




//Top-Level 'STREAMSelect' function
STREAM *STREAMSelect(ListNode *Streams, struct timeval *tv)
{
    TSelectSet *Set;
    STREAM *S=NULL;
    int result, UseEPOLL=FALSE;
    ListNode *Curr;

#ifdef HAVE_EPOLL
    if (ListSize(Streams) > 5) UseEPOLL=TRUE;
#endif

    Set=STREAMSelectInit(Streams, UseEPOLL);

    // if there's a stream with buffered data, then
    // STREAMSelectAddStreams will return it, and
    // we *must* useit, or risk that data lying in
    // the buffer forever
    S=STREAMSelectAddStreams(Set, Streams, UseEPOLL);

    //if there's no stream with buffered data, do the actual select
    if (! S)
    {
        result=STREAMSelectWait(Set, tv, UseEPOLL);
        if (result > 0)
        {
            Curr=ListGetNext(Streams);
            while (Curr)
            {
                if (STREAMSelectCheck(Set, (STREAM *) Curr->Item, UseEPOLL))
                {
                    S=Curr->Item;
                    ListMoveEnd(Curr);
                    break;
                }
                Curr=ListGetNext(Curr);
            }
        }
    }

    SelectSetDestroy(Set);

    return(S);
}

