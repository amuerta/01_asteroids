#define NOB_IMPLEMENTATION
#include "nob.h"

int main(int argc, char **argv)
{
	NOB_GO_REBUILD_URSELF(argc, argv);
	Nob_Cmd cmd = {0};
	bool selected = false;
	
	for(int i = 0; i < argc; i++) {
		if (strcmp(argv[i],"linux-native")==0) {
			cmd.count = 0;
			nob_cmd_append(&cmd, 
					"cc", 
					"-Wall", "-Wextra", 
					"-lraylib",
					"-lX11",
					"-lGL",
					"-lm",
					"-ggdb",
					"-o", 
					"main", "main.c"
					);
			selected = true;
		} 
		else if (strcmp(argv[i],"windows-native")==0) {
			cmd.count = 0;
			nob_cmd_append(&cmd, 
					"cc", 
					"-Wall", "-Wextra", 
					"-lraylib",
					"-lgdi32",
					"-lwinmm",
					"-o", 
					"main", "main.c"
					);
			selected = true;
		} 
		else if (strcmp(argv[i],"linux-to-windows")==0) {
			cmd.count = 0;
			nob_cmd_append(&cmd, 
					"x86_64-w64-mingw32-gcc", 
					"-o", "main.exe", "main.c",
					"-Wall", "-Wextra", 
					"-I./lib_extern/lib/raylib-5.5_win64_mingw-w64/include/", 
					"-L./lib_extern/lib/raylib-5.5_win64_mingw-w64/lib/",
					"-lraylib",
					"-lwinmm",
					"-lgdi32",
			);
			selected = true;
		}
	}

	if (!selected)
		nob_log(NOB_ERROR,"ENTER ONE OF THE BUILD OPTIONS: \n\n linux-native,\n windows-native, \n linux-to-windows.\n\n");

	if (!nob_cmd_run_sync(cmd)) return 1;


	for(int i = 0; i < argc; i++) {
		if (strcmp(argv[i],"run")==0) {
			cmd.count = 0;
			nob_cmd_append(&cmd, "./main");
			if (!nob_cmd_run_sync(cmd)) return 1;
		}

		if (strcmp(argv[i],"run-exe")==0) {
			cmd.count = 0;
			nob_cmd_append(&cmd, "./main.exe");
			if (!nob_cmd_run_sync(cmd)) return 1;
		}
	}

	return 0;
}
