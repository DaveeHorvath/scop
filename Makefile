NAME=triangle
CXX=c++
LIBS=/home/dhorvath/.libs
CXXFLAGS= -std=c++17 -Ofast -DDEBUG -g -I$(LIBS)/stb
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

SRC_DIR =src/
SRC_FILE=$(wildcard $(SRC_DIR)*.cpp)
SRC=$(addprefix $(SRC_DIR), $(SRC_FILE))
OBJ_DIR = obj/
OBJ=$(addprefix $(OBJ_DIR), $(notdir $(SRC:.cpp=.o)))
SHADER_DIR = shaders/
SHADERS=$(addprefix $(SHADER_DIR), shader.frag shader.vert)
SPV=$(addprefix $(SHADER_DIR), frag.spv vert.spv)

$(NAME): $(OBJ_DIR) $(OBJ) $(SPV)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJ) $(LDFLAGS)

run: $(NAME)
	@./triangle

$(SHADER_DIR)%.spv: $(SHADER_DIR)shader.%
	glslc $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp $(SRC_DIR)%.hpp
	$(CXX) $(CXXFLAGS) -c -o $(NAME) $< -o $@ 

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp
	$(CXX) $(CXXFLAGS) -c -o $(NAME) $< -o $@ 

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean $(NAME)

.PHONY: clean fclean re