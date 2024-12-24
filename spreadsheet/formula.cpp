#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

class Sheet;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

namespace {
class Formula : public FormulaInterface {
public:
    explicit Formula(std::string expression) try
        : ast_(ParseFormulaAST(expression)) {
    } catch (...) {
        throw FormulaException("");
    }
    
    Value Evaluate(const SheetInterface& sheet) const override {
        std::unordered_map< std::string, double > args;
        for (const Position& pos : GetReferencedCells()) {
            const CellInterface* cell = sheet.GetCell(pos);
            if (cell == nullptr) {
                args.emplace(pos.ToString(),0.0);
            }
            else {
                if (std::holds_alternative<double>(cell->GetValue())) {
                    args.emplace(pos.ToString(), std::get<double>(cell->GetValue()));
                }
                else if (std::holds_alternative<FormulaError>(cell->GetValue())) {
                    return FormulaError::Category::Value;
                }
                else {
                    std::string str = std::get<std::string>(cell->GetValue());
                    int value_int = 0;
                    try {
                        value_int = std::stoi(str);
                    }
                    catch (...) {
                        return FormulaError::Category::Value;
                    }
                    int length_value_int = 0;
                    while (value_int) {
                        value_int = value_int / 10;
                        length_value_int++;
                    }
                    if (length_value_int == int(str.size())) {
                        args.emplace(pos.ToString(), std::stoi(str) * 1.0);
                    }
                    else {
                        return FormulaError::Category::Value;
                    }
                }
            }
        }
        try {
            return ast_.Execute(args);
        }
        catch (const FormulaError& err) {
            return err;
        }
    }

    std::string GetExpression() const override {
        std::stringstream ss;
        ast_.PrintFormula(ss);
        return ss.str();
    }

    std::vector<Position> GetReferencedCells() const override {
        for (const auto& cell_position : ast_.GetCells()) {  
            if (!cell_position.IsValid()) {
                throw FormulaError::Category::Ref;
            }
        }
        std::set<Position> set_position{ ast_.GetCells().begin(), ast_.GetCells().end() };
        return { set_position.begin(), set_position.end() };  
    }

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}