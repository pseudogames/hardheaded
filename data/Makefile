
INCS=\
	$(patsubst %.ogg, %.h, \
	$(patsubst %.ttf, %.h, \
	$(patsubst %.png, %.h, \
	$(patsubst %.jpg, %.h, \
		$(wildcard *.png *.jpg *.ogg *.ttf) \
	))))

.PHONY: all data clean

all: data

data: $(INCS)

clean:
	rm -vf $(INCS)

%.h : %.ttf
	xxd -i $^ > $@
%.h: %.png
	xxd -i $^ > $@
%.h: %.jpg
	xxd -i $^ > $@
%.h: %.ogg
	xxd -i $^ > $@



