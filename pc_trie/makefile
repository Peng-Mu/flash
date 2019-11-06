

TARGET = aaa
INC = ./inc 
##CFILES = wholetrie_test.c ftrie.c pc_trie.c wholetrie.c
OBJS = wholetrie_test.o ftrie.o pc_trie.o wholetrie.o 
$(TARGET) : $(OBJS) 
	$(CC) -o $(TARGET) $(OBJS) 

##这里.h都放在该路径下的一个文件夹内，直接写.o与.h的依赖关系会报错，因为.h此时不在当前路径下面，\
  要么把所有的.h都删了(编译器隐含规则能找到)，要么就指定.h文件所在的文件夹，但是 -I $(INC)是不能删的。
wholetrie_test.o :  wholetrie_test.c ./inc/wholetrie.h ./inc/pc_trie.h ./inc/ftrie.h
	$(CC) -c wholetrie_test.c -I $(INC)
ftrie.o : ftrie.c ./inc/ftrie.h
	$(CC) -c ftrie.c -I $(INC)
pc_trie.o : pc_trie.c ./inc/pc_trie.h
	$(CC) -c pc_trie.c -I $(INC)
wholetrie.o : wholetrie.c ./inc/wholetrie.h ./inc/ftrie.h ./inc/pc_trie.h
	$(CC) -c wholetrie.c -I $(INC)

.PHONY:clean
clean :
	-rm $(TARGET) $(OBJS)
