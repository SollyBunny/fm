
build: fm prev
buildforce: fmforce prevforce

run:
	"./fm"
	@ echo exited

fmforce: fm
fm:
	cc "main.c" "folder.c" "window.c" -o "fm" -O4

prevforce: prev
prev:
	cc "preview.c" -o "preview" -lmagic -O4

	
