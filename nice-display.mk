
stripe=@for((i=0; i<`tput cols`; ++i)) do echo -n "="; done; echo

# named_stripe() - Print a "named stipe" i.e. '====..==== some name ====...===`
# arguments:
#	name - the "name" which will be printed in the stripe
#	name_color - the color of the name "name"
#	stripe_color - the color of the name "name"

define named_stripe=
	@cols=$$[`tput cols`/2 - `echo $(1) | wc -c`/2 - 2]; \
	echo -n -e $(3); for((i=0; i<cols; ++i)) do echo -n "="; done; echo -ne "\E[0m";\
	for((i=0; i<2; ++i)) do echo -n " "; done; \
	\
	echo -n -e $(2)"$(1)""\E[0m"; \
	for((i=0; i<2; ++i)) do echo -n " "; done; \
	echo -n -e $(3); for((i=0; i<cols; ++i)) do echo -n "="; done; echo -e "\E[0m"; 
endef
