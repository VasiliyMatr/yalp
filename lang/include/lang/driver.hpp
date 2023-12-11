#ifndef DRIVER_HPP
#define DRIVER_HPP

#include <memory>
#include <vector>

#ifndef yyFlexLexer
#include <FlexLexer.h>
#endif

#include <lang_parser.tab.hpp>

namespace yy {

class Driver final {
    using TType = parser::token_type;
    using INode = lang::InterfaceASTNode;
    using INodeUPtr = std::unique_ptr<INode>;

    FlexLexer *m_lexer = nullptr;

    std::vector<INodeUPtr> m_ast_nodes{};

    const lang::InterfaceASTNode *m_ast_root;

  public:
    template <class Node, typename... Args> Node *create(Args &&...args) {
        return dynamic_cast<Node *>(m_ast_nodes
                                       .emplace_back(std::make_unique<Node>(
                                           std::forward<Args>(args)...))
                                       .get());
    }

    parser::token_type yylex(parser::semantic_type *yylval) {
        auto ttype = static_cast<TType>(m_lexer->yylex());

        switch (ttype) {
        case TType::ID: {
            const auto *id = create<lang::ASTNode::Id>(m_lexer->YYText());

            yylval->emplace<const lang::ASTNode::Id *>(id);
            break;
        }
        case TType::FIXED_VAL: {
            const auto *fixed_val =
                create<lang::ASTNode::FixedVal>(std::atof(m_lexer->YYText()));

            yylval->emplace<const lang::ASTNode::FixedVal *>(fixed_val);
            break;
        }
        case TType::INT_VAL: {
            const auto *int_val =
                create<lang::ASTNode::IntVal>(std::atoi(m_lexer->YYText()));

            yylval->emplace<const lang::ASTNode::IntVal *>(int_val);
            break;
        }
        default:
            break; // no semantic value
        }

        return ttype;
    }

    void setASTRoot(const INode *ast_root) { m_ast_root = ast_root; }

    const INode *buildAST(std::istream &in) {
        yyFlexLexer lexer;
        lexer.switch_streams(&in);

        m_lexer = &lexer;

        parser parser{this};

        parser.parse();
        m_lexer = nullptr;

        return m_ast_root;
    }
};

} // namespace yy

#endif // DRIVER_HPP
