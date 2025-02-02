all:	voice_tracker

voice_tracker:	voice_tracker.c
	gcc voice_tracker.c -o voice_tracker

test:	voice_tracker
	./voice_tracker | sort

lua.test:	voice_tracker.lua
	luajit voice_tracker.lua | sort

clean:
	rm -rf voice_tracker *.?~

