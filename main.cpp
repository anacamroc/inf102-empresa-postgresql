#include <iostream>
#include <string>
#include <cstdlib>
#include <pqxx/pqxx>

using namespace std;

class Funcionario {
private:
    int matricula;
    string nome;
    double salario;

public:
    Funcionario() {
        matricula = 0;
        nome = "";
        salario = 0;
    }

    Funcionario(int matricula, string nome, double salario) {
        this->matricula = matricula;
        this->nome = nome;
        this->salario = salario;
    }

    int getMatricula() { return matricula; }
    string getNome() { return nome; }
    double getSalario() { return salario; }
};

class FuncionarioDAO {
private:
    pqxx::connection& conexao;

public:
    FuncionarioDAO(pqxx::connection& conn) : conexao(conn) {}

    void inserir(Funcionario f) {
        pqxx::work txn(conexao);

        txn.exec_params(
            "INSERT INTO funcionario (matricula, nome, salario) VALUES ($1, $2, $3)",
            f.getMatricula(),
            f.getNome(),
            f.getSalario()
        );

        txn.commit();
        cout << "\nFuncionario inserido com sucesso.\n";
    }

    void listar() {
        pqxx::work txn(conexao);

        pqxx::result res = txn.exec(
            "SELECT matricula, nome, salario FROM funcionario ORDER BY matricula"
        );

        cout << "\n=== FUNCIONARIOS ===\n";

        if (res.empty()) {
            cout << "Nenhum funcionario cadastrado.\n";
        }

        for (auto row : res) {
            cout << "Matricula: " << row["matricula"].as<int>() << endl;
            cout << "Nome: " << row["nome"].as<string>() << endl;
            cout << "Salario: R$ " << row["salario"].as<double>() << endl;
            cout << "--------------------\n";
        }

        txn.commit();
    }

    void buscar(int matricula) {
        pqxx::work txn(conexao);

        pqxx::result res = txn.exec_params(
            "SELECT matricula, nome, salario FROM funcionario WHERE matricula = $1",
            matricula
        );

        if (!res.empty()) {
            auto row = res[0];

            cout << "\nFuncionario encontrado:\n";
            cout << "Matricula: " << row["matricula"].as<int>() << endl;
            cout << "Nome: " << row["nome"].as<string>() << endl;
            cout << "Salario: R$ " << row["salario"].as<double>() << endl;
        } else {
            cout << "\nFuncionario nao encontrado.\n";
        }

        txn.commit();
    }

    void atualizarSalario(int matricula, double salario) {
        pqxx::work txn(conexao);

        pqxx::result res = txn.exec_params(
            "UPDATE funcionario SET salario = $1 WHERE matricula = $2",
            salario,
            matricula
        );

        txn.commit();

        if (res.affected_rows() > 0) {
            cout << "\nSalario atualizado com sucesso.\n";
        } else {
            cout << "\nFuncionario nao encontrado.\n";
        }
    }

    void excluir(int matricula) {
        pqxx::work txn(conexao);

        pqxx::result res = txn.exec_params(
            "DELETE FROM funcionario WHERE matricula = $1",
            matricula
        );

        txn.commit();

        if (res.affected_rows() > 0) {
            cout << "\nFuncionario removido com sucesso.\n";
        } else {
            cout << "\nFuncionario nao encontrado.\n";
        }
    }
};

int main() {
    try {
        const char* url = getenv("DATABASE_URL");

        if (url == nullptr) {
            cerr << "Erro: variavel DATABASE_URL nao definida.\n";
            cerr << "Use: export DATABASE_URL=\"sua_url_do_railway\"\n";
            return 1;
        }

        pqxx::connection conexao(url);

        if (!conexao.is_open()) {
            cerr << "Erro ao conectar ao banco de dados.\n";
            return 1;
        }

        cout << "\nConectado ao banco PostgreSQL com sucesso!\n";

        FuncionarioDAO dao(conexao);

        int opcao;

        do {
            cout << "\n===== MENU FUNCIONARIO =====\n";
            cout << "1 - Inserir funcionario\n";
            cout << "2 - Listar funcionarios\n";
            cout << "3 - Buscar funcionario\n";
            cout << "4 - Atualizar salario\n";
            cout << "5 - Excluir funcionario\n";
            cout << "0 - Sair\n";
            cout << "Opcao: ";
            cin >> opcao;

            switch (opcao) {
                case 1: {
                    int matricula;
                    string nome;
                    double salario;

                    cout << "Matricula: ";
                    cin >> matricula;

                    cin.ignore();

                    cout << "Nome: ";
                    getline(cin, nome);

                    cout << "Salario: ";
                    cin >> salario;

                    Funcionario f(matricula, nome, salario);
                    dao.inserir(f);

                    break;
                }

                case 2:
                    dao.listar();
                    break;

                case 3: {
                    int matricula;

                    cout << "Matricula: ";
                    cin >> matricula;

                    dao.buscar(matricula);

                    break;
                }

                case 4: {
                    int matricula;
                    double salario;

                    cout << "Matricula: ";
                    cin >> matricula;

                    cout << "Novo salario: ";
                    cin >> salario;

                    dao.atualizarSalario(matricula, salario);

                    break;
                }

                case 5: {
                    int matricula;

                    cout << "Matricula: ";
                    cin >> matricula;

                    dao.excluir(matricula);

                    break;
                }

                case 0:
                    cout << "\nEncerrando o sistema.\n";
                    break;

                default:
                    cout << "\nOpcao invalida.\n";
            }

        } while (opcao != 0);

    } catch (const exception& e) {
        cerr << "\nErro: " << e.what() << endl;
        return 1;
    }

    return 0;
}
