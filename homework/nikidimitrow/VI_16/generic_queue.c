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

int enqueue(QueueList *q, const void *data)
{
    struct node *newNode = (node *)malloc(sizeof(node));

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

    newNode->next = NULL;

    memcpy(newNode->data, data, q->memSize);

    if(q->sizeOfQueue == 0)
    {
        q->head = q->tail = newNode;
    }
    else
    {
        q->tail->next = newNode;
        q->tail = newNode;
    }

    q->sizeOfQueue++;
    return 0;
}

void dequeue(QueueList *q, void *data)
{
    if(q->sizeOfQueue > 0)
    {
        struct Node *temp = q->head;
        memcpy(data, temp->data, q->memSize);

        if(q->sizeOfQueue > 1)
        {
            q->head = q->head->next;
        }
        else
        {
            q->head = NULL;
            q->tail = NULL;
        }

        q->sizeOfQueue--;
        free(temp->data);
        free(temp);
    }
}
