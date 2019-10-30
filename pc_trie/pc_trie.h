#ifndef  __pc_trie_h__
#define  __pc_trie_h__

typedef struct s_PcNode
{
	int len;//value to indentify if the node is inherited
	int id;
} t_PcNode;

typedef struct s_PcSlot
{
	t_PcNode pc_node[4];
	struct s_PcSlot *next;
} t_PcSlot;

char* Bitmap_Create(int prefix_len);
t_PcNode* PcNode_Create(int num);
t_PcSlot** Slot_Create(void);
t_PcSlot* Slot_Add(t_PcSlot **slot_head, int slot_num);
void Slot_Delete(t_PcSlot **slot_head, int slot_num);
t_PcSlot* Slot_Locate(t_PcSlot **slot_head, int slot_num);
//t_PcSlot* Left_Slot(char *bitmap, t_PcSlot **slot_head, int bitmap_num);
//t_PcSlot* Right_Slot(char *bitmap, t_PcSlot **slot_head, int bitmap_num);
//t_PcSlot* Brother_Slot(char *bitmap, t_PcSlot **slot_head, int bitmap_num);
//t_PcSlot* Father_Slot(char *bitmap, t_PcSlot **slot_head, int bitmap_num);
int Count_One(char *bitmap, int start, int end);
t_PcNode* PcNode_Locate(char *bitmap, t_PcSlot **slot_head, t_PcNode *node_head, int num);
t_PcNode* Left_Child(char *bitmap, t_PcSlot **slot_head, t_PcNode *node_head, int num, int pre_one_cnt, int *curr_one_cnt);
t_PcNode* Right_Child(char *bitmap, t_PcSlot **slot_head, t_PcNode *node_head, int num, int pre_one_cnt, int *curr_one_cnt);
int PcTrie_Pfx_Num(char *data);
void PcTrie_Pfx_Add(int num, int id, int length, char *bitmap, t_PcSlot **slot_head, t_PcNode *node_head);
void PcTrie_Pfx_Deliver(int num, int id, int length, int one_cnt, char *bitmap, t_PcSlot **slot_head, t_PcNode *node_head);
void PcTrie_Pfx_Inherit(int num, char *bitmap, t_PcSlot **slot_head, t_PcNode *node_head);
void PcTrie_Add_Update(char *data, int id, int length, char *bitmap, t_PcSlot **slot_head, t_PcNode *node_head);
void PcTrie_Pfx_Delete(int num, char *bitmap, t_PcSlot **slot_head, t_PcNode *node_head);
int PcTrie_Pfx_Search(char *data, char *bitmap, t_PcSlot **slot_head, t_PcNode *node_head);


#endif

