### usar con 'make x01', 'make x02' ..... 'make x12', o 'make xs13'

.SUFFIXES:
.PRECIOUS: ejemplo%_exe ejemplo%s_exe

isdarwin   := $(filter Darwin,$(shell uname -s))
compilador := $(if $(isdarwin), clang++, g++)
flags      := -g -std=c++11 -pthread

x%: ejemplo%_exe
	./$<

ejemplo%_exe: ejemplo%.cpp
	$(compilador) $(flags)  -o $@ $<

xs%: ejemplo%s_exe
	./$<

ejemplo%s_exe: ejemplo%-s.cpp
	$(compilador) $(flags)  -o $@ $< scd.cpp

semaforo%_exe: ejemplo12_sem%.cpp
	$(compilador) $(flags)  -o $@ $< scd.cpp

prodconsLIFO_exe: prodconsLIFO.cpp
	$(compilador) $(flags)  -o $@ $< scd.cpp

prodconsFIFO_exe: prodconsFIFO.cpp
	$(compilador) $(flags)  -o $@ $< scd.cpp

clean:
	rm -f *_exe
