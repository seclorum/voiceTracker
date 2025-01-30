all:	voice_tracker

voice_tracker:	voice_tracker.c
	gcc voice_tracker.c -o voice_tracker

test:	voice_tracker
	./voice_tracker | sort

clean:
	rm -rf voice_tracker *.?~

