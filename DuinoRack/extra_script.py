Import("env", "projenv")

env.AddPostAction(
	"$BUILD_DIR/${PROGNAME}.elf",
	env.VerboseAction("avr-objdump -h -S $BUILD_DIR/${PROGNAME}.elf > $BUILD_DIR/${PROGNAME}.lss",
	"Creating $BUILD_DIR/${PROGNAME}.lss")
)
