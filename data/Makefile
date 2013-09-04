
OBJS=\
	$(patsubst %.ogg, %.o, \
	$(patsubst %.ttf, %.o, \
	$(patsubst %.png, %.o, \
	$(patsubst %.jpg, %.o, \
		$(wildcard *.png *.jpg *.ogg *.ttf) \
	))))

MIDS=$(patsubst %.o, %.c, $(OBJS))

.PHONY: all data clean

all: data

data: $(OBJS) all.h

clean:
	rm -vf $(OBJS) $(MIDS) all.h

%.c : %.ttf
	xxd -i $^ > $@
%.c: %.png
	xxd -i $^ > $@
%.c: %.jpg
	xxd -i $^ > $@
%.c: %.ogg
	xxd -i $^ > $@

%.o: %.c
	gcc $< -c -o $@

all.h: $(MIDS)
	sed -n 's/^\(.*\) = .*/extern \1;/p' $^ | tee $@


