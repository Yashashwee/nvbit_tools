TARGET := post-traces-processing

$(TARGET): post-traces-processing.cpp
	g++ -std=c++11 -o $@ $^
	g++ -o split_trace  split_trace.cpp

run: $(TARGET)
	./$(TARGET)

clean: 
	rm -f $(TARGET) *.o split_trace

