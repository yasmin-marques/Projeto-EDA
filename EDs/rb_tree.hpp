#ifndef RB_TREE_HPP
#define RB_TREE_HPP

#include <stack>        // usado no iterador
#include <utility>      // std::pair
#include <algorithm>    // std::max
#include <stdexcept>
#include <string>

#define RED false
#define BLACK true

// Estrutura que representa um nó da árvore Rubro-Negra
template <typename Key, typename Data>
struct rb_node {
    std::pair<Key, Data> key;    // par armazenado no nodo
    bool color;                  // cor do nó
    rb_node* m_left;             // ponteiro para o filho esquerdo
    rb_node* m_right;            // ponteiro para o filho direito
    rb_node* m_parent;           // pai do nó

    // Construtor do nó com chave e dado
    rb_node(const Key& k, const Data& d, bool c = RED,
            rb_node* l = nullptr, rb_node* r = nullptr, rb_node* p = nullptr)
        : key(std::make_pair(k, d)), color(c), m_left(l), m_right(r), m_parent(p) {}
};

// Classe genérica de uma árvore Rubro-Negra
template <typename Key, typename Data, typename Compare = std::less<Key>>
class red_black_tree {
private:
    rb_node<Key, Data>* m_root;         // raiz da árvore
    rb_node<Key, Data>* nil;            // nó nulo
    unsigned int m_size = 0;            // quantidade de elementos
    Compare m_compare;                   // objeto de comparação
    unsigned int num_comparisons = 0;   // número de comparações feitas
    unsigned int num_rotations = 0;      // número de rotações feitas

    // Libera memória de todos os nós recursivamente
    void _clear(rb_node<Key, Data>* p) {
        if(p != nil) {
            _clear(p->m_left);
            _clear(p->m_right);
            delete p;
        }
    }

    // Rotação simples para a esquerda
    void _left_rotate(rb_node<Key, Data>* p) {
        num_rotations++;
        rb_node<Key, Data>* u = p->m_right;
        p->m_right = u->m_left;
        num_comparisons++;
        if(u->m_left != nil) {
            u->m_left->m_parent = p;
        }

        u->m_parent = p->m_parent;
        num_comparisons++;
        if (p->m_parent == nil) m_root = u;
        else if (p == p->m_parent->m_left) {
            p->m_parent->m_left = u;
        }
        else p->m_parent->m_right = u;

        u->m_left = p;
        p->m_parent = u;
    }

    // Rotação simples para a direita
    void _right_rotate(rb_node<Key, Data>* p) {
        num_rotations++;
        rb_node<Key, Data>* u = p->m_left;
        num_comparisons++;
        p->m_left = u->m_right;
        if(u->m_right != nil) {
            u->m_right->m_parent = p;
        }

        u->m_parent = p->m_parent;
        num_comparisons++;
        if(p->m_parent == nil) m_root = u;
        else if (p == p->m_parent->m_left) {
            p->m_parent->m_left = u;
        }
        else p->m_parent->m_right = u;

        u->m_right = p;
        p->m_parent = u;
    }

    // Corrige as cores e estrutura após inserção
    void _insert_fixup(rb_node<Key, Data>* p) {
        while (p->m_parent->color == RED) {
            num_comparisons++;
            if (p->m_parent == p->m_parent->m_parent->m_left) {
                rb_node<Key, Data>* u = p->m_parent->m_parent->m_right;
                if (u->color == RED) {
                    // Caso 1
                    p->m_parent->color = BLACK;
                    u->color = BLACK;
                    p->m_parent->m_parent->color = RED;
                    p = p->m_parent->m_parent;
                } else {
                    if (p == p->m_parent->m_right) {
                        // Caso 2
                        p = p->m_parent;
                        _left_rotate(p);
                    }
                    // Caso 3
                    p->m_parent->color = BLACK;
                    p->m_parent->m_parent->color = RED;
                    _right_rotate(p->m_parent->m_parent);
                }
            } else {
                // mesma lógica, espelhada
                rb_node<Key, Data>* u = p->m_parent->m_parent->m_left;
                if (u->color == RED) {
                    p->m_parent->color = BLACK;
                    u->color = BLACK;
                    p->m_parent->m_parent->color = RED;
                    p = p->m_parent->m_parent;
                } else {
                    if (p == p->m_parent->m_left) {
                        p = p->m_parent;
                        _right_rotate(p);
                    }
                    p->m_parent->color = BLACK;
                    p->m_parent->m_parent->color = RED;
                    _left_rotate(p->m_parent->m_parent);
                }
            }
        }
        m_root->color = BLACK;
    }

    // Busca um nó com a chave k a partir do nó p
    rb_node<Key, Data>* _search(rb_node<Key, Data>* p, const Key& k) {
        while(p != nil && k != p->key.first) {
            num_comparisons++;
            if(m_compare(k, p->key.first)) p = p->m_left;
            else p = p->m_right;
        }
        return p;
    }

    // Retorna o menor nó da subárvore
    rb_node<Key, Data>* _minimum(rb_node<Key, Data>* p) {
        while(p->m_left != nil) p = p->m_left;
        return p;
    }

    // Corrige a árvore após remoção, garantindo propriedades
    void _remove_fixup(rb_node<Key, Data>* p) {
        while(p != m_root && p->color == BLACK) {
            if(p == p->m_parent->m_left) {
                rb_node<Key, Data>* u = p->m_parent->m_right;
                if(u->color == RED) {
                    // Caso 1
                    u->color = BLACK;
                    p->m_parent->color = RED;
                    _left_rotate(p->m_parent);
                    u = p->m_parent->m_right;
                }
                if(u->m_left->color == BLACK && u->m_right->color == BLACK) {
                    // Caso 2
                    u->color = RED;
                    p = p->m_parent;
                } else {
                    if (u->m_right->color == BLACK) {
                        // Caso 3
                        u->m_left->color = BLACK;
                        u->color = RED;
                        _right_rotate(u);
                        u = p->m_parent->m_right;
                    }
                    // Caso 4
                    u->color = p->m_parent->color;
                    p->m_parent->color = BLACK;
                    u->m_right->color = BLACK;
                    _left_rotate(p->m_parent);
                    p = m_root;
                }
            } else {
                // mesma lógica, espelhada
                rb_node<Key, Data>* u = p->m_parent->m_left;
                if(u->color == RED) {
                    u->color = BLACK;
                    p->m_parent->color = RED;
                    _right_rotate(p->m_parent);
                    u = p->m_parent->m_left;
                }
                if(u->m_right->color == BLACK && u->m_left->color == BLACK) {
                    u->color = RED;
                    p = p->m_parent;
                } else {
                    if (u->m_left->color == BLACK) {
                        u->m_right->color = BLACK;
                        u->color = RED;
                        _left_rotate(u);
                        u = p->m_parent->m_left;
                    }
                    u->color = p->m_parent->color;
                    p->m_parent->color = BLACK;
                    u->m_left->color = BLACK;
                    _right_rotate(p->m_parent);
                    p = m_root;
                }
            }
        }
        p->color = BLACK;
    }

    // Remove o nó da árvore
    void _remove(rb_node<Key, Data>* p) {
        rb_node<Key, Data>* u = p;
        rb_node<Key, Data>* q;
        bool u_original_color = u->color;

        if (p->m_left == nil) {
            q = p->m_right;
            _transplant(p, p->m_right);
        } else if (p->m_right == nil) {
            q = p->m_left;
            _transplant(p, p->m_left);
        } else {
            u = _minimum(p->m_right);
            u_original_color = u->color;
            q = u->m_right;
            if (u->m_parent == p) {
                q->m_parent = u;
            } else {
                _transplant(u, u->m_right);
                u->m_right = p->m_right;
                u->m_right->m_parent = u;
            }
            _transplant(p, u);
            u->m_left = p->m_left;
            u->m_left->m_parent = u;
            u->color = p->color;
        }

        delete p;
        // se o nó removido era preto, pode ter quebrado a regra de balanceamento
        if (u_original_color == BLACK) _remove_fixup(q);
    }

    // Substitui o nó u por p na árvore
    void _transplant(rb_node<Key, Data>* u, rb_node<Key, Data>* p) {
        if(u->m_parent == nil) m_root = p;  // u era a raiz
        else if(u == u->m_parent->m_left) { 
            u->m_parent->m_left = p;
        }
        else { 
            u->m_parent->m_right = p;
        }
        p->m_parent = u->m_parent;
    }

    // Função auxiliar recursiva para percorrer a árvore em ordem e preencher um vetor com os pares
    void in_order(rb_node<Key, Data>* node, std::vector<std::pair<Key, Data>>& result) const {
        if (!node) return;
        in_order(node->m_left, result);
        if (!node->key.first.empty()) { 
            result.emplace_back(node->key.first, node->key.second);
        }
        in_order(node->m_right, result);
    }

public:
    // Construtor
    red_black_tree()
        : m_root(nullptr),
          nil(new rb_node<Key, Data>(Key(), Data(), BLACK)) {
        m_root = nil;
    }

    // Destrutor: libera todos os nós e o sentinela
    ~red_black_tree() {
        _clear(m_root);
        delete nil;
    }

    // Retorna true se a chave estiver na árvore
    bool contains(const Key& k) { return _search(m_root, k) != nil; }

    // Verifica se a árvore está vazia
    bool empty() const { return m_size == 0; }

    // Retorna o número de elementos da árvore
    unsigned int size() const { return m_size; }

    // Retorna o número de rotações realizadas
    unsigned int rotations() const { return num_rotations; }

    // Retorna o número de comparações realizadas
    unsigned int comparisons() const { return num_comparisons; }

    // Insere um novo nó com a chave e o dado especificados.
    // Se a chave já existir, a inserção é ignorada.
    void insert(const Key& k, const Data& d) {
        rb_node<Key, Data>* current = m_root;
        rb_node<Key, Data>* m_parent = nil;

        while (current != nil) {
            m_parent = current;
            num_comparisons++;
            if (m_compare(k, current->key.first)) {
                current = current->m_left;
            } else {
                num_comparisons++;
                if (m_compare(current->key.first, k)) {
                    current = current->m_right;
                } else {
                    return;         // a chave já existe, não insere
                }
            }
        }

        rb_node<Key, Data>* new_node = new rb_node<Key, Data>(k, d, RED, nil, nil, m_parent);
        if(m_parent == nil) {
            m_root = new_node;
        } else {
            num_comparisons++;
            if(m_compare(k, m_parent->key.first)) {
                m_parent->m_left = new_node;
            } else {
                m_parent->m_right = new_node;
            }
        }

        m_size++; 
        _insert_fixup(new_node);    // Corrige a estrutura e cores se necessário
    }

    // Remove uma chave da árvore
    void remove(const Key& k) {
        rb_node<Key, Data>* p = _search(m_root, k);
        if(p != nil) {
            _remove(p);
            m_size--;
        }
    }

    // Busca e retorna referência para o dado associado a chave
    Data& search(const Key& k) {
        rb_node<Key, Data>* p = _search(m_root, k);
        if (p == nil) throw std::out_of_range("Key not found");
        return p->key.second;
    }

    // Atualiza o dado de uma chave já existente
    void at(const Key& k, const Data& d) {
        rb_node<Key, Data>* p = _search(m_root, k);
        num_comparisons++;
        if(p != nil) p->key.second = d;
    }

    // Remove todos os elementos da árvore
    void clear() {
        _clear(m_root);
        m_root = nil;
        m_size = 0;
    }

    // Retorna o nome da estrutura (usado no relatório/saída)
    std::string name() const {
        return "Árvore Rubro-Negra";
    }

    // Retorna todos os elementos da árvore como um vetor de pares, em ordem crescente das chaves
    std::vector<std::pair<Key, Data>> all_elements() const {
        std::vector<std::pair<Key, Data>> result;
        in_order(m_root, result);
        return result;
    }

    // Classe iterator para percorrer a árvore (in-order)
    class iterator {
    private:
        rb_node<Key, Data>* m_current; // nó atual da iteração
        rb_node<Key, Data>* nil;       // nó sentinela

    public:
        // Construtor
        iterator(rb_node<Key, Data>* root, rb_node<Key, Data>* nil)
            : nil(nil), m_current(root) {}

        // Avança para o próximo nó em ordem
        iterator& operator++() {
            if (m_current->m_right != nil) {
                // vai para o menor da subárvore direita
                m_current = m_current->m_right;
                while (m_current->m_left != nil)
                    m_current = m_current->m_left;
            } else {
                // sobe ate encontrar um pai em que o nó atual é filho esquerdo
                rb_node<Key, Data>* p = m_current->m_parent;
                while (p != nil && m_current == p->m_right) {
                    m_current = p;
                    p = p->m_parent;
                }
                m_current = p;
            }
            return *this;
        }

        // Verifica se dois iteradores são diferentes
        bool operator!=(const iterator& other) const {
            return m_current != other.m_current;
        }

        // Acesso ao par (chave, dado)
        std::pair<Key, Data>& operator*() const { return m_current->key; }
        std::pair<Key, Data>* operator->() const { return &(m_current->key); }
    };

    // Retorna o início da travessia (menor chave)
    iterator begin() {
        rb_node<Key, Data>* p = m_root;
        while (p->m_left != nil) p = p->m_left;
        return iterator(p, nil);
    }

    // Retorna o fim da travessia (nil)
    iterator end() { return iterator(nil, nil); }
};

#endif