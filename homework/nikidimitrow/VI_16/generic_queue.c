struct Node
{
  void *someData;
  struct Node *nextNode;
};

struct QueueList
{
    int size;
    size_t memorySize;
    node *head;
    node *tail;
};

int  enqueue(QueueList *, const void *);
void dequeue(QueueList *, void *);

int enqueue(QueueList *q, const void *someData)
{
    struct Node *newNode = (node *)malloc(sizeof(node));

    if(newNode == NULL)
    {
        return -1;
    }

    newNode->data = malloc(q->memSize);

    if(newNode->data == NULL)
    {
        free(newNode);
        return -1;
    }

    newNode->nextNode = NULL;

    memcpy(newNode->someData, someData, q->memSize);

    if(q->sizeOfQueue == 0)
    {
        q->head = q->tail = newNode;
    }
    else
    {
        q->tail->nextNode = newNode;
        q->tail = newNode;
    }

    q->sizeOfQueue++;
    return 0;
}

void dequeue(QueueList *q, void *someData)
{
    if(q->sizeOfQueue > 0)
    {
        struct Node *tempNode = q->head;
        memcpy(someData, temp->someData, q->memSize);

        if(q->sizeOfQueue > 1)
        {
            q->head = q->head->nextNode;
        }
        else
        {
            q->head = NULL;
            q->tail = NULL;
        }

        q->sizeOfQueue--;
        free(tempNode->someData);
        free(tempNode);
    }
}
