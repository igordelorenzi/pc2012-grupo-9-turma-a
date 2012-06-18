# Makefile

CC = gcc
CFLAGS = -Wall

PROJECT_NAME = seq
SRCS = seq.c
OBJS = $(SRCS:.c=.o)

$(PROJECT_NAME): $(OBJS)
	@echo Linkando objetos para o binário \'$@\'
	$(CC) -o $@ $^
	@echo

%.o: %.c
	@echo Compilando arquivo objeto: $@
	$(CC) -c $< $(CFLAGS) -o $@
	@echo

clean:
	@echo Limpando arquivos
	-@rm -f *.o *.d $(PROJECT_NAME) *~
	@echo

mrproper: clean
	-@rm -rf $(PROJECT_NAME)
	@echo Está tudo limpo!
	@echo

remade:
	$(MAKE) clean
	$(MAKE)

-include $(SRC:.c=.d)

%.d: %.c
	@$(CC) $< -MM -MT '$*.o $*.d ' -MD $(CFLAGS)

# End of Makefile
