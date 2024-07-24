#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <cstdlib>

// Definición de los tipos de tokens
enum TipoToken {
    PREPROCESADOR, PALABRA_CLAVE, IDENTIFICADOR, NUMERO, CADENA, OPERADOR, SIMBOLO, DESCONOCIDO
};

// Estructura para representar un token
struct Token {
    TipoToken tipo;
    std::string valor;
    int linea;
};

// Estructura para representar un error
enum TipoError {
    ERROR_SINTACTICO, ERROR_SEMANTICO
};

struct Error {
    TipoError tipo;
    std::string mensaje;
    int linea;
};

// Función para realizar el análisis léxico
std::vector<Token> analizarLexico(const std::string& codigo) {
    std::vector<Token> tokens;
    std::regex patronesToken(R"((#winasqa)|\b(yahillu|piq|suchu|kaqlla|llapan|chayq|qilluq|chirtu|kuti)\b|[a-zA-Z_][a-zA-Z0-9_]*|\b\d+\b|"[^"]*"|<<|==|<=|>=|!=|&&|\|\||\S)");
    std::smatch coincidencia;
    std::string codigoRestante = codigo;
    int linea = 1;

    while (std::regex_search(codigoRestante, coincidencia, patronesToken)) {
        Token token;
        token.valor = coincidencia.str();
        token.linea = linea;

        if (coincidencia.str() == "#winasqa") token.tipo = PREPROCESADOR;
        else if (coincidencia.str() == "yahillu" || coincidencia.str() == "piq" ||
                 coincidencia.str() == "suchu" || coincidencia.str() == "kaqlla" ||
                 coincidencia.str() == "llapan" || coincidencia.str() == "chayq" ||
                 coincidencia.str() == "qilluq" || coincidencia.str() == "chirtu" ||
                 coincidencia.str() == "kuti") token.tipo = PALABRA_CLAVE;
        else if (std::regex_match(coincidencia.str(), std::regex("[a-zA-Z_][a-zA-Z0-9_]*"))) token.tipo = IDENTIFICADOR;
        else if (std::regex_match(coincidencia.str(), std::regex("\\d+"))) token.tipo = NUMERO;
        else if (std::regex_match(coincidencia.str(), std::regex("\"[^\"]*\""))) token.tipo = CADENA;
        else if (std::regex_match(coincidencia.str(), std::regex("<<|==|<=|>=|!=|&&|\\|\\|"))) token.tipo = OPERADOR;
        else if (std::regex_match(coincidencia.str(), std::regex("\\S"))) token.tipo = SIMBOLO;
        else token.tipo = DESCONOCIDO;

        tokens.push_back(token);
        linea += std::count(coincidencia.prefix().first, coincidencia.prefix().second, '\n');
        codigoRestante = coincidencia.suffix().str();
    }

    return tokens;
}

// Función para realizar el análisis sintáctico
std::vector<Error> analizarSintaxis(const std::vector<Token>& tokens) {
    std::vector<Error> errores;
    size_t indice = 0;

    // Función para verificar si el token actual es del tipo esperado
    auto esperar = [&](TipoToken tipoEsperado, const std::string& mensajeError) {
        if (indice < tokens.size() && tokens[indice].tipo == tipoEsperado) {
            indice++;
        } else {
            errores.push_back({ERROR_SINTACTICO, mensajeError, (indice < tokens.size() ? tokens[indice].linea : -1)});
        }
    };

    // Función para verificar si el valor del token actual es el esperado
    auto esperarValor = [&](const std::string& valorEsperado, const std::string& mensajeError) {
        if (indice < tokens.size() && tokens[indice].valor == valorEsperado) {
            indice++;
        } else {
            errores.push_back({ERROR_SINTACTICO, mensajeError, (indice < tokens.size() ? tokens[indice].linea : -1)});
        }
    };

    // Analizar la estructura del programa
    if (tokens[indice].valor == "#winasqa") {
        esperar(PREPROCESADOR, "Se esperaba '#winasqa'");
        esperar(SIMBOLO, "Se esperaba '<'");
        esperar(PALABRA_CLAVE, "Se esperaba 'yahillu'");
        esperar(SIMBOLO, "Se esperaba '>'");
        esperar(PALABRA_CLAVE, "Se esperaba 'piq'");
        esperar(PALABRA_CLAVE, "Se esperaba 'suchu'");
        esperar(PALABRA_CLAVE, "Se esperaba 'kaqlla'");
        esperar(SIMBOLO, "Se esperaba ';'");
    }

    // Analizar la función principal
    esperar(PALABRA_CLAVE, "Se esperaba 'llapan'");
    esperar(PALABRA_CLAVE, "Se esperaba 'chayq'");
    esperar(SIMBOLO, "Se esperaba '('");
    esperar(SIMBOLO, "Se esperaba ')'");
    esperar(SIMBOLO, "Se esperaba '{'");

    // Analizar el contenido de la función principal
    while (indice < tokens.size() && tokens[indice].tipo != SIMBOLO && tokens[indice].valor != "}") {
        if (tokens[indice].tipo == PALABRA_CLAVE && tokens[indice].valor == "llapan") {
            esperar(PALABRA_CLAVE, "Se esperaba 'llapan'");
            esperar(IDENTIFICADOR, "Se esperaba un identificador despues de 'llapan'");
            if(tokens[indice].tipo == SIMBOLO && tokens[indice].valor == "="){
                esperar(SIMBOLO, "Se esperaba '='");
                if(tokens[indice].tipo == NUMERO){
                    esperar(NUMERO, "Se esperaba un número");
                    if (tokens[indice].tipo == SIMBOLO && tokens[indice].valor == "+") {
                        esperar(SIMBOLO, "Se esperaba '+'");
                        if (tokens[indice].tipo == NUMERO) {
                            esperar(NUMERO, "Se esperaba un número");
                        } else if (tokens[indice].tipo == IDENTIFICADOR) {
                            esperar(IDENTIFICADOR, "Se esperaba un identificador");
                        }
                    }
                } else if (tokens[indice].tipo == IDENTIFICADOR){
                    esperar(IDENTIFICADOR, "Se esperaba un identificador");
                    if(tokens[indice].tipo == SIMBOLO && tokens[indice].valor == "+"){
                        esperar(SIMBOLO, "Se esperaba '+'");
                        if(tokens[indice].tipo == NUMERO){
                            esperar(NUMERO, "Se esperaba un número");
                        } else if (tokens[indice].tipo == IDENTIFICADOR){
                            esperar(IDENTIFICADOR, "Se esperaba un identificador");
                        }
                    }
                } else {
                    errores.push_back({ERROR_SINTACTICO, "Se esperaba un numero o un identificador", tokens[indice].linea});
                }
            }
            esperar(SIMBOLO, "Se esperaba ';'");
        } else if (tokens[indice].tipo == PALABRA_CLAVE && tokens[indice].valor == "qilluq") {
            esperar(PALABRA_CLAVE, "Se esperaba 'qilluq'");
            while (tokens[indice].tipo == OPERADOR && tokens[indice].valor == "<<") {
                esperar(OPERADOR, "Se esperaba '<<'");
                if (tokens[indice].tipo == CADENA || tokens[indice].tipo == IDENTIFICADOR || tokens[indice].tipo == NUMERO) {
                    indice++;
                } else {
                    break;
                }
            }
            if(tokens[indice].tipo == SIMBOLO && tokens[indice].valor == ";"){
                esperar(SIMBOLO, "Se esperaba ';'");
            } else if (tokens[indice].valor == "chirtu") {
                esperar(PALABRA_CLAVE, "Se esperaba 'chirtu'");
                esperar(SIMBOLO, "Se esperaba ';'");
            }
        } else if (tokens[indice].tipo == PALABRA_CLAVE && tokens[indice].valor == "kuti") {
            esperar(PALABRA_CLAVE, "Se esperaba 'kuti'");
            esperar(NUMERO, "Se esperaba un número después de 'kuti'");
            esperar(SIMBOLO, "Se esperaba ';'");
        } else {
            errores.push_back({ERROR_SINTACTICO, "Sintaxis desconocida", tokens[indice].linea});
            indice++;
        }
    }

    esperar(SIMBOLO, "Se esperaba '}'");

    return errores;
}

// Función para realizar el análisis semántico
std::vector<Error> analizarSemantica(const std::vector<Token>& tokens) {
    std::vector<Error> errores;
    std::unordered_set<std::string> variables_definidas;
    size_t indice = 0;
    bool directiva_present = false;

    // Verificar la presencia de la directiva al inicio
    if (tokens.size() >= 8 && 
        tokens[0].tipo == PREPROCESADOR && tokens[0].valor == "#winasqa" &&
        tokens[1].tipo == SIMBOLO && tokens[1].valor == "<" &&
        tokens[2].tipo == PALABRA_CLAVE && tokens[2].valor == "yahillu" &&
        tokens[3].tipo == SIMBOLO && tokens[3].valor == ">" &&
        tokens[4].tipo == PALABRA_CLAVE && tokens[4].valor == "piq" &&
        tokens[5].tipo == PALABRA_CLAVE && tokens[5].valor == "suchu" &&
        tokens[6].tipo == PALABRA_CLAVE && tokens[6].valor == "kaqlla" &&
        tokens[7].tipo == SIMBOLO && tokens[7].valor == ";") {
        directiva_present = true;
        indice = 8;  // Saltar los tokens de la directiva
    }

    while (indice < tokens.size()) {
        if (tokens[indice].tipo == PALABRA_CLAVE && tokens[indice].valor == "qilluq") {
            if (!directiva_present) {
                errores.push_back({ERROR_SEMANTICO, "Uso de 'qilluq' sin la directiva '#winasqa <yahillu> piq suchu kaqlla;' presente al inicio", tokens[indice].linea});
            }
        }

        if (tokens[indice].tipo == PALABRA_CLAVE && tokens[indice].valor == "llapan") {
            indice++;
            if (tokens[indice].tipo == IDENTIFICADOR) {
                variables_definidas.insert(tokens[indice].valor);
                indice++;
                if (tokens[indice].tipo == SIMBOLO && tokens[indice].valor == "=") {
                    indice++;
                    if (tokens[indice].tipo == IDENTIFICADOR) {
                        if (variables_definidas.find(tokens[indice].valor) == variables_definidas.end()) {
                            errores.push_back({ERROR_SEMANTICO, "Variable no definida: " + tokens[indice].valor, tokens[indice].linea});
                        }
                    }
                    indice++;
                }
            }
        } else if (tokens[indice].tipo == IDENTIFICADOR) {
            if (variables_definidas.find(tokens[indice].valor) == variables_definidas.end()) {
                errores.push_back({ERROR_SEMANTICO, "Variable no definida: " + tokens[indice].valor, tokens[indice].linea});
            }
            indice++;
        } else {
            indice++;
        }
    }

    return errores;
}


// Función para generar código C++ a partir de los tokens
std::string generarCodigoCpp(const std::vector<Token>& tokens) {
    std::string codigoCpp; // = "#include <iostream>\nusing namespace std;\nint main() {\n";

    for (const auto& token : tokens) {
        if(token.tipo == PREPROCESADOR && token.valor == "#winasqa"){
            codigoCpp += "#include ";
        } else if (token.tipo == PALABRA_CLAVE && token.valor == "yahillu") {
            codigoCpp += "iostream";
        } else if (token.tipo == PALABRA_CLAVE && token.valor == "piq") {
            codigoCpp += "using ";
        } else if (token.tipo == PALABRA_CLAVE && token.valor == "suchu") {
            codigoCpp += "namespace ";
        } else if (token.tipo == PALABRA_CLAVE && token.valor == "kaqlla") {
            codigoCpp += "std";
        } else if (token.tipo == PALABRA_CLAVE && token.valor == "llapan") {
            codigoCpp += "int ";
        } else if (token.tipo == PALABRA_CLAVE && token.valor == "chayq") {
            codigoCpp += "main ";
        } else if (token.tipo == IDENTIFICADOR) {
            codigoCpp += token.valor;
        } else if (token.tipo == NUMERO) {
            codigoCpp += token.valor;
        } else if (token.tipo == SIMBOLO && token.valor == ">") {
            codigoCpp += ">\n";
        } else if (token.tipo == SIMBOLO && token.valor == "=") {
            codigoCpp += " = ";
        } else if (token.tipo == SIMBOLO && token.valor == "{") {
            codigoCpp += "{\n";
        } else if (token.tipo == SIMBOLO && token.valor == ";") {
            codigoCpp += ";\n";
        } else if (token.tipo == SIMBOLO) {
            codigoCpp += token.valor;
        } else if (token.tipo == PALABRA_CLAVE && token.valor == "qilluq") {
            codigoCpp += "cout";
        } else if (token.tipo == CADENA) {
            codigoCpp += token.valor;
        } else if (token.tipo == OPERADOR && token.valor == "<<") {
            codigoCpp += " << ";
        } else if (token.tipo == PALABRA_CLAVE && token.valor == "chirtu") {
            codigoCpp += "endl";
        } else if (token.tipo == PALABRA_CLAVE && token.valor == "kuti") {
            codigoCpp += "return ";
        } else {
            codigoCpp += token.valor;
        }
    }

    std::cout << codigoCpp << std::endl;
    
    return codigoCpp;
}

// Función para leer el contenido de un archivo
std::string leerArchivo(const std::string& nombreArchivo) {
    std::ifstream archivo(nombreArchivo);
    if (!archivo) {
        throw std::runtime_error("Error al abrir el archivo.");
    }
    std::string contenido((std::istreambuf_iterator<char>(archivo)), std::istreambuf_iterator<char>());
    return contenido;
}

// Función para escribir el código C++ a un archivo
void escribirArchivo(const std::string& nombreArchivo, const std::string& codigoCpp) {
    std::ofstream archivo(nombreArchivo);
    if (!archivo) {
        throw std::runtime_error("Error al escribir el archivo.");
    }
    archivo << codigoCpp;
}

// Función principal que realiza el análisis léxico, sintáctico, semántico y generación de código
void analizarCodigo(const std::string& archivoEntrada, const std::string& archivoSalidaCpp, const std::string& archivoSalidaEjecutable) {
    try {
        std::string codigo = leerArchivo(archivoEntrada);

        // Análisis léxico
        std::vector<Token> tokens = analizarLexico(codigo);

        // Análisis sintáctico
        std::vector<Error> erroresSintaxis = analizarSintaxis(tokens);
        if (!erroresSintaxis.empty()) {
            for (const auto& error : erroresSintaxis) {
                std::cerr << "Error Sintactico: " << error.mensaje << " en la linea " << error.linea << std::endl;
            }
            return;
        }

        // Análisis semántico
        std::vector<Error> erroresSemanticos = analizarSemantica(tokens);
        if (!erroresSemanticos.empty()) {
            for (const auto& error : erroresSemanticos) {
                std::cerr << "Error Semantico: " << error.mensaje << " en la linea " << error.linea << std::endl;
            }
            return;
        }

        // Generación de código C++
        std::string codigoCpp = generarCodigoCpp(tokens);
        escribirArchivo(archivoSalidaCpp, codigoCpp);

        // Compilación del código C++
        std::string comando = "g++ " + archivoSalidaCpp + " -o " + archivoSalidaEjecutable;
        int resultado = system(comando.c_str());
        if (resultado != 0) {
            throw std::runtime_error("Error al compilar el código.");
        }
        
        std::cout << "-----------------------------------------------------" << std::endl;
        std::cout << "Codigo compilado exitosamente. Ejecutando programa..." << std::endl;
        std::cout << "-----------------------------------------------------" << std::endl;
        // Ejecución del código compilado
        comando = archivoSalidaEjecutable;
        system(comando.c_str());

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void analizadorLexico (std::string archivo) {
    std::ifstream file(archivo);
    if (!file) {
        std::cerr << "No se pudo abrir el archivo 'codigo.txt'" << std::endl;
        return;
    }

    std::string code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    
    std::vector<Token> tokens = analizarLexico(code);

    for (const auto& token : tokens) {
        std::cout << "Token: " << token.valor << "\tTipo: ";
        switch (token.tipo) {
            case PREPROCESADOR: std::cout << "Preprocesador"; break;
            case PALABRA_CLAVE: std::cout << "Palabra clave"; break;
            case IDENTIFICADOR: std::cout << "Identificador"; break;
            case NUMERO: std::cout << "Numero"; break;
            case CADENA: std::cout << "Cadena"; break;
            case OPERADOR: std::cout << "Operador"; break;
            case SIMBOLO: std::cout << "Simbolo"; break;
            case DESCONOCIDO: std::cout << "Desconocido"; break;
            default: std::cout << "Desconocido"; break;
        }
        std::cout << std::endl;
    }
}

void analizadorSintactico(const std::string& archivo) {
    try {
        std::string codigo = leerArchivo(archivo);

        // Análisis léxico
        std::vector<Token> tokens = analizarLexico(codigo);

        // Análisis sintáctico
        std::vector<Error> erroresSintaxis = analizarSintaxis(tokens);
        if (!erroresSintaxis.empty()) {
            for (const auto& error : erroresSintaxis) {
                std::cerr << "Error Sintactico: " << error.mensaje << " en la linea " << error.linea << std::endl;
            }
            return;
        }

        std::cout << "Codigo sin errores sintacticos." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void analizadorSemantico(const std::string& archivo) {
    try {
        std::string codigo = leerArchivo(archivo);

        // Análisis léxico
        std::vector<Token> tokens = analizarLexico(codigo);

        // Análisis sintáctico
        std::vector<Error> erroresSintaxis = analizarSintaxis(tokens);
        if (!erroresSintaxis.empty()) {
            for (const auto& error : erroresSintaxis) {
                std::cerr << "Error Sintactico: " << error.mensaje << " en la linea " << error.linea << std::endl;
            }
            return;
        }

        // Análisis semántico
        std::vector<Error> erroresSemanticos = analizarSemantica(tokens);
        if (!erroresSemanticos.empty()) {
            for (const auto& error : erroresSemanticos) {
                std::cerr << "Error Semantico: " << error.mensaje << " en la linea " << error.linea << std::endl;
            }
            return;
        }

        std::cout << "Codigo sin errores sintacticos ni semanticos." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}


int main() {
    int opcion;
    do {
        std::cout << "-----------------------------------------------------" << std::endl;
        std::cout << "Menu de Opciones:" << std::endl;
        std::cout << "1. Analizador Lexico" << std::endl;
        std::cout << "2. Analizador Sintactico" << std::endl;
        std::cout << "3. Analizador Semantico" << std::endl;
        std::cout << "4. Generar Codigo C++" << std::endl;
        std::cout << "0. Salir" << std::endl;
        std::cout << "Opcion: ";
        std::cin >> opcion;
        switch (opcion) {
            case 1:
                std::cout << "-----------------------------------------------------" << std::endl;
                analizadorLexico("codigo.txt");
                break;
            case 2:
                std::cout << "-----------------------------------------------------" << std::endl;
                analizadorSintactico("codigo.txt");
                break;
            case 3:
                std::cout << "-----------------------------------------------------" << std::endl;
                analizadorSemantico("codigo.txt");
                break;
            case 4:
                std::cout << "-----------------------------------------------------" << std::endl;
                analizarCodigo("codigo.txt", "codigo.cpp", "codigo");
                break;
            case 0:
                std::cout << "Saliendo..." << std::endl;
                break;
            default:
                std::cout << "Opcion Invalida" << std::endl;
                break;
        }
    } while (opcion != 0);
    return 0;
}