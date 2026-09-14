/*========================== LUCA MOBILIA'S PROGRAM ===========================================*/

/*
        For simplicity, I decided to work with cubic coordinates.
        Doing so made it easier to calculate distances.

        Some methods for finding distances/coordinates were inspired by explanations given at the following site: https://www.redblobgames.com/grids/hexagons
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>       //floor(), fmax(), fmin()
#include <string.h>     //strcmp()
#include <limits.h>     //INT_MAX
#include <stdbool.h>    //bool type

///////////////// DATA STRUCTURES ////////////////
//AIR ROUTE
typedef struct
{
    int x;                 //destination column
    int y;                 //destination row
} AirRoute;

//HEXAGON
typedef struct
{
    //cubic coordinates set
    int q;
    int r;
    int s;

    int cost;               //exit cost
    int n_routes;           //number of routes starting from it

    AirRoute* routes[5];    //pointer to air routes (struct type)
} Hexagon;

//NODE for QUEUE
typedef struct
{
    int r;          //node row
    int c;          //node column

    int cost;       //cost to reach that node
} Node;

//PRIORITY QUEUE
typedef struct
{
    Node *heap;     //the priority queue is made up of a set of nodes
    int size;       //partial queue size
    int maxdim;     //maximum queue size (rows*cols)
} PQ;

////////////////////////////////////////////////

//////////////// GLOBAL VARIABLES /////////////
//MATRIX TO REPRESENT THE HEXAGON STRUCTURE
Hexagon** map;
int rows;
int cols;
////////////////////////////////////////////////

//////////////// FUNCTIONS /////////////////////
void init();                                                //initializes (or reinitializes) the <nrows>x<ncols> map
void change_cost(int c, int r, int v, int radius);          //modifies the cost
void toggle_air_route (int c1, int r1, int c2, int r2);     //adds/removes air routes
int travel_cost(int cp, int rp, int cd, int rd);            //returns minimum cost

void free_map();
void cube_coord(Hexagon *e, int row, int col);              //calculates the cubic coordinates
void oddr_coord(int *row, int *col, int q, int r);          //calculates the "standard" coordinates
int DistHexagons(Hexagon* a, Hexagon* b);                   //calculates the distance between two hexagons
void delete_route(int x, int y, int index);                 //deletes the route from the array
void add_route(int x1, int y1, int x2, int y2);             //adds the route to the array

PQ* create_pq(int maxdim);                                  //creates the queue node
void node_swap(Node* a, Node* b);                           //swaps nodes
void push(PQ* queue, int r, int c, int cost);               //inserts a new node into the heap
Node pop(PQ* queue);                                        //extracts the node from the heap
void free_queue(PQ* queue);                                 //frees the queue
int dijkstra(int rs, int cs, int rd, int cd);               //shortest path search algorithm

//}//////////////////////////////////////////////


int main()
{
    char string[17];    //toggle_air_route: longest specification, 16 characters (+1) (=17)
    bool start=false;   //indicates that no init has been done yet

    while(fscanf(stdin, "%16s", string)==1)     //loop until error or EOF
    {
        if(strcmp(string, "init")==0)           //if it reads init
        {
            //auxiliary variables
            int n1;
            int n2;

            if(fscanf(stdin, "%d %d", &n1, &n2)==2 && n1>0 && n2>0) //if the passed numbers are greater than zero, I accept the specification
            {
                if(start==true) free_map();    //if I have already done at least one init, then I must delete the pre-existing map

                //store the passed numbers, the dimensions of the base rectangle
                cols=n1;
                rows=n2;

                init();     //execute init

                start=true;     //I have done at least one init
            }
        }
        else if(strcmp(string, "change_cost")==0)   //if it reads change_cost
        {
            //auxiliary variables
            int x;
            int y;
            int v;
            int radius;

            if(fscanf(stdin, "%d %d %d %d", &x, &y, &v, &radius)==4)
            {
                change_cost(x, y, v, radius);
            }
        }
        else if(strcmp(string, "toggle_air_route")==0)  //if it reads toggle_air_route
        {
            //auxiliary variables
            int x1;
            int y1;
            int x2;
            int y2;

            if(fscanf(stdin, "%d %d %d %d", &x1, &y1, &x2, &y2)==4)
            {
                toggle_air_route(x1, y1, x2, y2);
            }
        }
        else if(strcmp(string, "travel_cost")==0)   //if it reads travel_cost
        {
            //auxiliary variables
            int sum;
            int x1;
            int y1;
            int x2;
            int y2;

            if(fscanf(stdin, "%d %d %d %d", &x1, &y1, &x2, &y2)==4)
            {
                sum=travel_cost(x1, y1, x2, y2);    //save the cost in a support variable
                printf("%d\n", sum);                //print the result to the screen
            }
        }
    }

    free_map();     //free the map when I reach the end of the file

    return 0;
}

void free_map()
{
    for(int i=0; i<rows; i++)
    {
        for(int j=0; j<cols; j++)
        {
            if(map[i][j].n_routes!=0)    //if I had previously created any air routes
            {
                for(int r=0; r<map[i][j].n_routes; r++) { free( map[i][j].routes[r] ); }   //free the air routes (necessary because they are dynamic)
            }
        }
        free(map[i]);
    }
    free(map);
}

void cube_coord(Hexagon *e, int row, int col)
{
    e->q = col-(row-(row%2))/2;
    e->r = row;
    e->s = -(e->q)-(e->r);
}

void oddr_coord(int *row, int *col, int q, int r)
{
    *col = q+(r-(r%2))/2;
    *row = r;
}

void init()
{
    //allocate map
    map=malloc(sizeof(Hexagon*)*rows);          //allocate an array of pointers to Hexagon (rows)

    for(int i=0; i<rows; i++)
    {
        map[i]=malloc(sizeof(Hexagon)*cols);    //allocate columns (Hexagon, in turn)
        for(int j=0; j<cols; j++)   //scroll through columns
        {
            map[i][j].cost=1;           //initialize all hexagons to cost 1
            map[i][j].n_routes=0;       //initialize the number of routes to 0
            //function to store cubic coordinates
            cube_coord(&map[i][j], i, j);
            for(int k=0; k<5; k++) { map[i][j].routes[k]=NULL; }     //since they are pointers, they point to NULL initially
        }
    }

    //successful, OK
    printf("OK\n");
}

int DistHexagons(Hexagon* a, Hexagon* b)
{
    return ( (abs((a->q)-(b->q)) + abs((a->r)-(b->r)) + abs((a->s)-(b->s)))/2 );      //exploit cubic coordinates, adaptation of Manhattan distance
}

void change_cost(int col, int ro, int v, int radius)
{
    //v between -10 and 10
    //radius is positive
    //the passed hexagon is valid (exists)
    if((-10<=v && v<=10) && (radius>0) && (0<=ro && ro<rows) && (0<=col && col<cols))
    {
        //loop necessary to calculate the hexagons within the radius
        //I must force the cast, otherwise error
        for(int dq = -radius; dq <= radius; dq++) //variations that q assumes
        {
            int dr_min, dr_max;
            //needed to calculate the extremes to analyze everything
            dr_min=fmax(-radius, -dq-radius);
            dr_max=fmin(radius, -dq+radius);

            for(int dr = dr_min; dr <= dr_max; dr++)
            {
                //coordinates of the hexagon to analyze
                int re=-1;
                int ce=-1;

                                    //q coordinate        //r coordinate
                oddr_coord(&re, &ce, (map[ro][col].q+dq), (dr+ro));

                //if the hexagon to observe exists
                if((0<=re && re<rows) && (0<=ce && ce<cols))
                {
                    int distance=DistHexagons(&map[ro][col], &map[re][ce]);
                    int new_cost;       //support variable, I cannot declare it unsigned since it could also take a negative value
                                                                                        //explicit float type so I don't always get 0
                    new_cost=map[re][ce].cost+floor( v*fmax(0,(((float)radius-(float)distance)/(float)radius)));

                    //by specification, the new cost must be between 0 and 100
                    if(new_cost<0) map[re][ce].cost=0;
                    else if (new_cost>100) map[re][ce].cost=100;
                    else map[re][ce].cost=new_cost;
                }
            }
        }

        //since everything was successful
        printf("OK\n");
    }
    //if at least one of the specifications is not good
    else printf("KO\n");
}

void delete_route(int x, int y, int index)
{
    for(int i=index; i<map[x][y].n_routes-1; i++)
    {
        //shift the array to the left, starting from the element I have to delete
        map[x][y].routes[i]=map[x][y].routes[i+1];
    }

    //free the last space of the array (since it is a pointer and therefore dynamically allocated)
    free(map[x][y].routes[map[x][y].n_routes-1]);

    //decrease the route counter
    map[x][y].n_routes--;
}

void add_route(int x1, int y1, int x2, int y2)
{
    //index of the air routes array
    //remember that the number of routes starts from 0, every time a value is added, the variable is incremented immediately after
    //so the first route is at index 0, the second at index 1, and so on
    int index=map[x1][y1].n_routes;

    //allocate memory for a cell of the array, since it's a pointer
    map[x1][y1].routes[index]=malloc(sizeof(AirRoute));

    //assign destinations
    map[x1][y1].routes[index]->y=x2;        //remember that routes[].y is the row coordinate
    map[x1][y1].routes[index]->x=y2;        //remember that routes[].x is the column coordinate

    //increment the route counter
    map[x1][y1].n_routes++;
}

void toggle_air_route (int c1, int r1, int c2, int r2)
{
    //if both hexagons exist
    if( (0<=r1 && r1<rows) && (0<=c1 && c1<cols) && (0<=r2 && r2<rows) && (0<=c2 && c2<cols) )
    {
        bool found=false;        //used to interrupt the loop the moment I find the route, if it already exists
        int i;              //used to store the index of the found route

        //check if the route already exists
        //I have to check every connection
        for(i=0; i<map[r1][c1].n_routes && found==false; i++)
        {
            //if I verify the condition, then the route already exists
            if((map[r1][c1].routes[i]->y == r2) && (map[r1][c1].routes[i]->x == c2)) found=true;
        }

        //if the route exists
        if(found==true)
        {
            delete_route(r1, c1, i);        //delete it
            printf("OK\n");         //success message
        }
        //otherwise I verify that the number of routes starting from hexagon 1 is less than 5 (if there are already 5 it's not good)
        else if(map[r1][c1].n_routes<5)
        {
            add_route(r1, c1, r2, c2);      //add the new route
            printf("OK\n");         //success message
        }
        //or it means I cannot add other routes
        else printf("KO\n");
    }
    //if at least one of the two hexagons does not exist
    else printf("KO\n");
}

PQ* create_pq(int maxdim)
{
    PQ *queue=malloc(sizeof(PQ));
    if(queue)       //if allocation successful
    {
        queue->heap=malloc(sizeof(Node)*maxdim);
        if(!queue->heap)        //if allocation fails
        {
            free(queue);
            return NULL;
        }
        queue->size=0;
        queue->maxdim=maxdim;
        return queue;
    }
    return NULL;
}

void node_swap(Node* a, Node* b)
{
    Node tmp=*a;
    *a=*b;
    *b=tmp;
}

void push(PQ* queue, int r, int c, int cost)
{
    if(queue->maxdim > queue->size)         //if there is enough space
    {
        int i=queue->size;      //new index, insert it as the last element of the queue

        //add data to the heap
        queue->heap[i].r=r;
        queue->heap[i].c=c;
        queue->heap[i].cost=cost;

        while(i>0)      //restore the Min-heap property when adding the element
        {
            int p=(i-1)/2;      //p is the parent
            if(queue->heap[i].cost >= queue->heap[p].cost) break;   //if the parent has cost less than or equal to the child, the property is already respected
            node_swap(&queue->heap[i], &queue->heap[p]);            //otherwise swap nodes
            i=p;
        }
        queue->size++;
    }
    return;     //if there are more elements than the Heap can hold
}

Node pop(PQ* queue)
{
    Node tmp=queue->heap[0];    //head
    queue->size--;              //decrease the size because I have to delete an element

    if(queue->size>0)       //if there is at least one other element
    {
        queue->heap[0]=queue->heap[queue->size];    //the last value becomes the first

        //restore MinHeap properties
        int i=0;
        while(1)
        {
            int left=(2*i)+1;       //left child index
            int right=(2*i)+2;      //right child index
            int middle=i;

            //left/right child exists && its cost is lower than current, swap positions
            if( (left<queue->size) && (queue->heap[left].cost < queue->heap[middle].cost) ) middle=left;
            if( (right<queue->size) && (queue->heap[right].cost < queue->heap[middle].cost) ) middle=right;

            if(middle==i) break;        //if the current node is already the smallest, it is already a MinHeap

            node_swap(&queue->heap[i], &queue->heap[middle]);   //swap with the smallest child

            i=middle;       //middle is the new position to compare
        }
    }

    return tmp;
}

void free_queue(PQ* queue)
{
    if(queue)
    {
        free(queue->heap);
        free(queue);
    }
}

int dijkstra(int rs, int cs, int rd, int cd)
{
    //initialize the distance matrix and the matrix that tells me if a hexagon has been visited
    int dist[rows][cols];
    bool visited[rows][cols];
    for(int i=0; i<rows; i++)
    {
        for(int j=0; j<cols; j++)
        {
            dist[i][j]=INT_MAX;     //maximum value an integer can take
            visited[i][j]=false;
        }
    }

    //create the priority queue
    PQ* queue=create_pq(rows*cols);
    if(!queue) return -1;       //if allocation fails

    dist[rs][cs]=0;             //distance from starting node is zero
    push(queue, rs, cs, 0);     //add starting hexagon to the priority queue

    while(queue->size!=0)   //stops when the queue is empty
    {
        Node current=pop(queue);    //returns the node with the lowest cost
        int r=current.r;
        int c=current.c;

        if((0<=r && r<rows) && (0<=c && c<cols) && visited[r][c]==false)    //continue only if the extracted node has not been visited yet
        {
            visited[r][c]=true;    //mark it as visited

            if(r==rd && c==cd)  //if the visited node is the destination one
            {

                int result=dist[r][c];       //support variable because before returning I must deallocate dist[][]

                //free the allocated memory
                free_queue(queue);

                return result;
            }

            //check air routes
            Hexagon* hex=&map[r][c];    //support variable
            for(int i=0; i<hex->n_routes; i++)
            {
                int nc=(hex->routes[i])->x;      //x is the column
                int nr=(hex->routes[i])->y;      //y is the row

                //if the destination node has not yet been visited && the route cost is greater than zero
                if(visited[nr][nc]==false && hex->cost>0 && (0<=nr && nr<rows) && (0<=nc && nc<cols) )
                {
                    //support variable for the new total cost
                                //cost of the path up to the current node
                                            //exit cost from the current node
                    int new_dist=dist[r][c]+hex->cost;

                    if(new_dist<dist[nr][nc])       //if the new cost is less than the accumulated total cost (better path than before)
                    {
                        dist[nr][nc]=new_dist;      //update the path cost up to that moment
                        push(queue, nr, nc, new_dist);  //add the node to the Heap
                    }
                }
            }

            //check neighbors by land
            if(hex->cost>0)         //if I can leave the node
            {
                //the radius to observe is 1, since I have to look at the adjacent nodes
                //I use the same code present in change_cost (comments are there)
                for(int dq=-1; dq<= 1; dq++)
                {
                    int dr_min=fmax(-1, -dq-1);
                    int dr_max=fmin(1, -dq+1);

                    for(int dr=dr_min; dr<=dr_max; dr++)
                    {
                        int re=-1, ce=-1;
                        oddr_coord(&re, &ce, (dq+hex->q), (dr+hex->r));

                        //if the neighbor is in the map
                        if((0<=re && re<rows) && (0<=ce && ce<cols) && visited[re][ce]==false)
                        {
                            //support variable for the new total cost
                                        //cost of the path up to the current node
                                                    //exit cost from the current node
                            int new_dist=dist[r][c]+hex->cost;

                            if(new_dist<dist[re][ce])       //if the new cost is less than the accumulated total cost (better path than before)
                            {
                                dist[re][ce]=new_dist;      //update the path cost up to that moment
                                push(queue, re, ce, new_dist);  //add the node to the Heap
                            }
                        }
                    }
                }
            }
        }
    }

    //free the queue
    free_queue(queue);

    //the destination is not reachable
    return -1;
}

int travel_cost(int cp, int rp, int cd, int rd)
{
    //if hexagon exists
    if( (0<=rp && rp<rows) && (0<=cp && cp<cols) && (0<=rd && rd<rows) && (0<=cd && cd<cols) )
    {
        //if the destination hexagon is the same as the starting one
        if( (cp==cd) && (rp==rd) ) return 0;

        //pass the value of the starting hexagon and the arriving one
        return dijkstra(rp, cp, rd, cd);
    }

    //if the hexagon doesn't exist
    return -1;
}
