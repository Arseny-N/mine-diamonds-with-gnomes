include nice-display.mk



CFLAGS:=-g -Wall -O0
CLIBS=-lncurses


objs = random.o print_graphs.o graphs.o
headers = graphs.h  list.h  print_graphs.h  random.h  stddef.h  types.h  utils.h  vertex_list.h


main: main.c $(objs) $(headers)




# Nice output
%.o : %.c %.h
	$(call named_stripe,$@,"\E[2m","\E[2m")
	$(CC) $(CFLAGS) $(CLIBS)  -c $^
%.bin : %.c $(objs) $(headers)
	$(call named_stripe,$@)
	$(CC) $(CFLAGS) $(CLIBS) -o $@  $^
% : %.c $(objs) $(headers)
	$(call named_stripe,$@)
	$(CC) $(CFLAGS) $(CLIBS) -o $@  $^
	 


view:
	xdot graph.0.dot 2>/dev/null &
	xdot graph.p0.dot 2>/dev/null &
	xdot graph.pt0.dot 2>/dev/null &
	
PHONY += view
.PHONY: $(PHONY)
