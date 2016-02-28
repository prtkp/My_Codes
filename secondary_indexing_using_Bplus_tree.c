/*-------------------------------------------------------------
Program description        :              secondary indexing using B+ Tree in c programming language
Acknowledgements           :              In this code, if you encounter any error, please contact me.
--------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include<string.h>

#define order 4

long int id = 0;  /* a variable which automatically */ 

/* structure for node of the B+ Tree */
typedef struct BTnode {
	 long int nodeId;    /* id of node */
   long int keys[order-1]; /* key values */
   long int childrenId[order];   /*child address for internal node and offset for leaf node */
   long int parentId;                   /*id of parent */
   int num_keys;                        /* no of keys in parent */
   bool is_leaf;                         /* is this a leaf or internal node */
} BTnode;


void insert_into_parent(FILE *outFile, BTnode * left, long int key, BTnode * right);
void split_node(FILE *outFile, BTnode *old_node, int ctr, long int key, BTnode *right);

/* structure for tracing the id of root */
typedef struct rootId {
	long int id;
} rootId;

rootId rid;    /* this variable always hold the node id of root */
BTnode * create_tnode( void ) {
	BTnode * tnode = malloc(sizeof(BTnode));
    if (tnode == NULL) {
		printf("Memory insufficient\n");	exit(0);
	}
   tnode->nodeId = ++id;
   tnode->parentId = -1; 
	tnode->is_leaf = false;
	tnode->num_keys = 0;
	return tnode;
}


/* function to search the record in database */
long int search( FILE *outFile, long int key ) {
	int i = 0, j = 0;
  long int k;
  rootId rid;
  BTnode *node = NULL;
  node = (BTnode*)malloc(sizeof(BTnode));
       rewind(outFile);
       fread(&rid, sizeof(rootId), 1, outFile);
       fseek(outFile, sizeof(rootId) + sizeof(BTnode) * (rid.id-1) ,SEEK_SET);
       fread(node, sizeof(BTnode), 1, outFile);

	BTnode * c = node;
	while (!c->is_leaf) {
		j = 0;
		while (j < c->num_keys) {
			if (key >= c->keys[j]) j++;
			else break;
		}
			 k = c->childrenId[i];
          fseek(outFile, sizeof(rootId) + sizeof(BTnode) * (k-1) ,SEEK_SET);
          fread(c, sizeof(BTnode), 1, outFile);
	}
	for (i = 0; i < c->num_keys; i++){
		if (c->keys[i] == key)     /* if key exist */
     return c->childrenId[i];
  }
	if (i == c->num_keys);     /* if key doesn't exist */
		return -1;
}


/* Insert the key, if exceed order then split in two halves */
void split_node(FILE *outFile, BTnode * old_node, int ctr, long int key, BTnode * right) {

	int i, j, split;
   long int k;
	BTnode * new_node, * child;
	long int temp_keys[order], temp_offset[order+1];
   
	for (i = 0, j = 0; i < old_node->num_keys + 1; i++, j++) {   /* copy keys in temp location */
		if (j == ctr + 1) j++;
		temp_offset[j] = old_node->childrenId[i];
	}

	for (i = 0, j = 0; i < old_node->num_keys; i++, j++) {       /* copy offsets/childrenId in temp location */
		if (j == ctr) j++;
		temp_keys[j] = old_node->keys[i];
	}

	temp_offset[ctr + 1] = right->nodeId;                     /* insert the child key into parent */
	temp_keys[ctr] = key;
   split = order;
	
   if (split % 2 == 0)                                       /* now split the parent in two halves */
	  split = split / 2;
	else
	  split = (split / 2) + 1;
     
	/* create the new node for new parent */
   new_node = create_tnode();
	 old_node->num_keys = 0; 
   rewind(outFile);
   
	for (i = 0; i < split - 1; i++) {                         //copy the left half keys and childrenId
		old_node->childrenId[i] = temp_offset[i];
		old_node->keys[i] = temp_keys[i];
		old_node->num_keys++;
	}
	old_node->childrenId[i] = temp_offset[i];                 //copy the middle key
	k = temp_keys[split - 1];                           //again check whether this key will go up and split or not
	
  for (++i, j = 0; i < order; i++, j++) {                    //copy the right half keys
		new_node->childrenId[j] = temp_offset[i];
		new_node->keys[j] = temp_keys[i];
		new_node->num_keys++;
	}

	new_node->childrenId[j] = temp_offset[i];
   long int chid;
	new_node->parentId = old_node->parentId;
	fseek(outFile, sizeof(rootId) + sizeof(BTnode) * (new_node->nodeId-1) ,SEEK_SET);
	fwrite(new_node,sizeof(BTnode), 1, outFile);
	fseek(outFile, sizeof(rootId) + sizeof(BTnode) * (old_node->nodeId-1) ,SEEK_SET);
	fwrite(old_node,sizeof(BTnode), 1, outFile);	
	child = (BTnode*)malloc(sizeof(BTnode));
  for (i = 0; i <= new_node->num_keys; i++) {
      chid = new_node->childrenId[i];
      rewind(outFile);
      fseek(outFile, sizeof(rootId) + sizeof(BTnode) * (chid-1) ,SEEK_SET);
      fread(child, sizeof(BTnode), 1, outFile);
      
		  child->parentId = new_node->nodeId;
      fseek(outFile, sizeof(rootId) + sizeof(BTnode) * (chid-1) ,SEEK_SET);
      fwrite(child,sizeof(BTnode), 1, outFile);
	}
   free(child);
	 return insert_into_parent(outFile, old_node, k, new_node);
}

/* connect the nodes with its parent */
void insert_into_parent(FILE *outFile, BTnode* left, long int key, BTnode* right) {

	BTnode * parent;
   long int parId;
   if(left->parentId == -1){
      BTnode * root = create_tnode();
 	    root->keys[0] = key;
	    root->childrenId[0] = left->nodeId;
	    root->childrenId[1] = right->nodeId;
	    root->num_keys++;
	    root->parentId = -1;
	    left->parentId = root->nodeId;
	    right->parentId = root->nodeId;
      rid.id = root->nodeId;
       
       rewind(outFile);
       fwrite(&rid, sizeof(rootId), 1, outFile);
       fseek(outFile, sizeof(rootId) + sizeof(BTnode) * (left->nodeId-1) ,SEEK_SET);
       fwrite(left, sizeof(BTnode), 1, outFile);
	     fseek(outFile, sizeof(rootId) + sizeof(BTnode) * (right->nodeId-1) ,SEEK_SET);
       fwrite(right, sizeof(BTnode), 1, outFile);
       fseek(outFile, sizeof(rootId) + sizeof(BTnode) * (root->nodeId-1) ,SEEK_SET);
       fwrite(root, sizeof(BTnode), 1, outFile);
       rewind(outFile); 
	    return;
	}
   /* read the parent */
   rewind(outFile);
   parent = (BTnode*)malloc(sizeof(BTnode));
   fseek(outFile, sizeof(rootId) + sizeof(BTnode) * (left->parentId-1) ,SEEK_SET);
   fread(parent, sizeof(BTnode), 1, outFile);
   
   int ctr = 0;
   
  /* find the pointer to left child */
	while (ctr <= parent->num_keys &&	parent->childrenId[ctr] != left->nodeId)
		ctr++;

	if (parent->num_keys < order - 1){
      int i;

	   for (i = parent->num_keys; i > ctr; i--) {               //shift the keys in the parent
		   parent->childrenId[i + 1] = parent->childrenId[i];
		   parent->keys[i] = parent->keys[i - 1];
	   }
	   parent->childrenId[ctr + 1] = right->nodeId;             //insert the key came from child into parent
	   parent->keys[ctr] = key;
	   parent->num_keys++;     //increment the no of keys.

       fseek(outFile, sizeof(rootId) + sizeof(BTnode) * (left->nodeId-1) ,SEEK_SET);
       fwrite(left, sizeof(BTnode), 1, outFile);
	     fseek(outFile, sizeof(rootId) + sizeof(BTnode) * (right->nodeId-1) ,SEEK_SET);
       fwrite(right, sizeof(BTnode), 1, outFile);
       fseek(outFile, sizeof(rootId) + sizeof(BTnode) * (parent->nodeId-1) ,SEEK_SET);
       fwrite(parent, sizeof(BTnode), 1, outFile);
       //rewind(outFile);
       
       free(parent);
       return;
       }
	/* split a node in order to preserve the B+ tree properties. */
	fseek(outFile, sizeof(rootId) + sizeof(BTnode) * (left->nodeId-1) ,SEEK_SET);
   fwrite(left, sizeof(BTnode), 1, outFile);
   rewind(outFile);
   rootId r ;
   fread(&r, sizeof(rootId), 1, outFile);
   
   return split_node(outFile, parent, ctr, key, right); //split internal node
}



/* split the leaf if size exceeds */
void split_leaf(FILE *outFile, BTnode * leaf, long int key, long int offset) {

	BTnode * new_leaf;
	long int temp_keys[order], temp_offset[order+1], new_key;
	int ctr = 0, split, i, j;
	
  new_leaf = create_tnode();
	new_leaf->is_leaf = true;

	while (ctr < order - 1 && leaf->keys[ctr] < key) //find the pos in node
		ctr++;

	for (i = 0, j = 0; i < leaf->num_keys; i++, j++) {                //shift the keys
		if (j == ctr) 
           j++;
		temp_keys[j] = leaf->keys[i];
		temp_offset[j] = leaf->childrenId[i];
	}

	temp_keys[ctr] = key;           //insert
	temp_offset[ctr] = offset;

	leaf->num_keys = 0;
  
  split = order - 1;
	if (split % 2 == 0)            //for odd order
	  split = split / 2;
	else      //for even order
	  split = (split / 2) + 1;

	for (i = 0; i < split; i++) {            //insert in 1st node
		leaf->childrenId[i] = temp_offset[i];
		leaf->keys[i] = temp_keys[i];
		leaf->num_keys++;
	}

	for (i = split, j = 0; i < order; i++, j++) {       //make halves and insert into both, half half keys
		new_leaf->childrenId[j] = temp_offset[i];        //insert in 2nd node
		new_leaf->keys[j] = temp_keys[i];
		new_leaf->num_keys++;
	}

	new_leaf->childrenId[order - 1] = leaf->childrenId[order - 1];   
	leaf->childrenId[order - 1] = new_leaf->nodeId;          //connect left with right child

	for (i = leaf->num_keys; i < order - 1; i++)
		 leaf->childrenId[i] = -1;
	for (i = new_leaf->num_keys; i < order - 1; i++)     //make null 2nd half of 1st node 
		 new_leaf->childrenId[i] = -1;

   
   new_leaf->parentId = leaf->parentId;
	 new_key = new_leaf->keys[0];
   rewind(outFile);
   fseek(outFile, sizeof(rootId) + sizeof(BTnode) * (leaf->nodeId-1) ,SEEK_SET);
   fwrite(leaf, sizeof(BTnode), 1, outFile);
	 fseek(outFile, sizeof(rootId) + sizeof(BTnode) * (new_leaf->nodeId-1) ,SEEK_SET);
   fwrite(new_leaf, sizeof(BTnode), 1, outFile);
	return insert_into_parent(outFile, leaf, new_key, new_leaf);
}

//function to insert the records into B+ Tree
void insert( FILE *outFile, long int key, long int offset ){
       rewind(outFile);
       if(fgetc(outFile) == EOF){
            BTnode * root = create_tnode();
            rid.id = root->nodeId;
            root->is_leaf = true;
            root->parentId =  -1;
            root->keys[0] = key;
            root->num_keys++;
            root->childrenId[0] = offset;
            rewind(outFile);
            fwrite(&rid, sizeof(rootId), 1, outFile);   //rid is id of root
            fwrite(root, sizeof(BTnode), 1, outFile);
            return;
            }
       BTnode * node, *leaf;
       rootId rid;
       node = (BTnode*)malloc(sizeof(BTnode));
       rewind(outFile);
       fread(&rid, sizeof(rootId), 1, outFile);
       fseek(outFile, sizeof(rootId) + sizeof(BTnode) * (rid.id-1) ,SEEK_SET);
       fread(node, sizeof(BTnode), 1, outFile);       //node is root
       
       rewind(outFile);
       leaf = node;     

       int i = 0, j = 0; long int k;
	     while (leaf->is_leaf != true) {
             i = 0;        //for non leaf node
		         while(i < leaf->num_keys)
             {
                 if (key >= leaf->keys[i]) i++;
			           else break;     
             }    
                      
          k = leaf->childrenId[i];
          rewind(outFile);
          fseek(outFile, sizeof(rootId) + sizeof(BTnode) * (k-1) ,SEEK_SET);
          fread(leaf, sizeof(BTnode), 1, outFile);
	    } //end of while 1
                   
       /* if leaf has space then insert */
	    if (leaf->num_keys < order - 1) {  //is there any vacant place
          int i, insertion_point = 0;
	       while (insertion_point < leaf->num_keys && leaf->keys[insertion_point] < key) //find the pos in node
	     	     insertion_point++;

	       for (i = leaf->num_keys; i > insertion_point; i--) { //shift the keys and insert the new key in array
		        leaf->keys[i] = leaf->keys[i - 1];
		        leaf->childrenId[i] = leaf->childrenId[i - 1];
	       }
	       leaf->keys[insertion_point] = key;
	       leaf->childrenId[insertion_point] = offset;
	       leaf->num_keys++;
        
        rewind(outFile);
        fseek(outFile, sizeof(rootId) + sizeof(BTnode) * (leaf->nodeId-1) ,SEEK_SET);
        fwrite(leaf, sizeof(BTnode), 1, outFile);
        free(leaf);
        return;
	}
	/* leaf must be split if size exceeds */
	return split_leaf(outFile, leaf, key, offset);
}     
   


/* main function */
int main() {
    int choice, flag1 = 1;
    
    printf("\n\tSecondary indexing on a database file using B+ Tree\n");
    char ch;
    FILE * fp, *outFile;
    BTnode * root = NULL;
	  int  range2;
    long int input;
    fp = fopen("data.txt", "r");      //open the input file
    outFile = fopen("output.dat","w");
    while(1){
    printf("\n====================================\n");
    printf("Enter your choice of operation\n 1 for indexing of input file\n 2 for searching a record from file\n");
    printf(" 5 for exit\n");
    printf("====================================\n");
    scanf("%d",&choice);
    if(choice == 1 && flag1 == 1){                          //insert the records
          flag1 = 0;
          if (fp == NULL) {
			    printf("File can't be opened...if the file doesn't exist or check the permissions\n");
			    exit(1);
		     }
         long int x = 0;
   
		 
         while (1) {              //insert the records and do indexing
               fscanf(fp, "%ld", &input);
               ch = fgetc(fp);
               x = ftell(fp);
               insert(outFile, input, x);

               while(ch != '\n')
                        ch = fgetc(fp);
               ch = fgetc(fp);
               if(ch == EOF)
                     break; 
               else
                     fseek(fp, -1, SEEK_CUR);     
               }
         printf("\tIndexing has been done successfully.\n");
      }
      else if(choice == 1 && flag1 == 0){                   //warning
            printf("\tinsertion has been already done.\n");
            }
      else if(choice == 2 && flag1 == 0){                  //search the records
            
            long int key, offset;
            printf("Enter the key value\n");
            scanf("%ld",&key);
            offset = search(outFile, key );
            printf("\t Searching the record...\n", key);
            if(offset != -1){
                  fseek(fp,offset, SEEK_SET);
                  while(ch != '\n'){
                     ch = fgetc(fp);
                     printf("%c",ch);
                     }
            }
            else
               printf(" Record not found\n");
     }
     else if(choice == 2 && flag1 == 1){                   //warning
          printf("Indexing is not done yet...first insert the records\n");
          }
     else if(choice == 5)     /* quit the program */
               exit(0);
     else              /* repeat */
         printf("\n!!! wrong choice....Try again\n");
     }//end of while
	return 0;
}
