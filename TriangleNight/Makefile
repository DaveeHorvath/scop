NAME=triangle
CXX=c++
LIBS=/home/dhorvath/.libs
CXXFLAGS= -std=c++17 -Ofast -DDEBUG -g -I$(LIBS)/stb
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

SRC=main.cpp
# SRC=test.cpp
OBJ=$(SRC:.cpp=.o)
SHADERS=shader.frag shader.vert
SPV=frag.spv vert.spv

test: $(NAME)
	./triangle

%.spv: shader.%
	glslc $< -o $@

$(NAME): $(OBJ) $(SPV)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJ) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $(NAME) $< -o $@ 

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean $(NAME)

.PHONY: clean fclean re