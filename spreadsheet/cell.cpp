#include "cell.h"

#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <variant>


Cell::Cell(Sheet& sheet) : sheet_(sheet) {
}
Cell::~Cell() = default;

class Cell::Impl {
public:
    Impl() = default;
    virtual ~Impl() = default;
    virtual void Set(std::string text) = 0;
    virtual Value GetValue() const = 0;
    virtual std::string GetText() const = 0;
    virtual bool IsFormula() const = 0;
};

class Cell::EmptyImpl : public Impl {
public:
    EmptyImpl() = default;
    void Set(std::string text) override { }
    Value GetValue() const override { return ""; }
    std::string GetText() const override { return ""; }
    bool IsFormula() const override { return false; }
};

class Cell::TextImpl : public Impl {
public:
    TextImpl() = default;

    TextImpl(std::string text) :text_(std::move(text)) {};

       void Set(std::string text) override {
           text_ = std::move(text);
       }

    Value GetValue() const override {
        return text_[0] == ESCAPE_SIGN ? text_.substr(1) : text_;
    }

    std::string GetText() const override { return text_; }

    bool IsFormula() const override { return false; }

private:
    std::string text_;
};

class Cell::FormulaImpl : public Impl {
public:

    explicit FormulaImpl(SheetInterface& sheet, std::string text) try
        :sheet_(sheet), formula_(ParseFormula(text)){
    }
    catch (const FormulaException& fe) {
        throw FormulaException(fe.what());
    };

     void Set(std::string text) override {
         try {
             formula_ = ParseFormula(text);
         }
         catch (const FormulaException& fe) {
             throw FormulaException(fe.what()); 
         }
     }

    Value GetValue() const override {
        using namespace std::literals;
        if (cache_.has_value()) {
            return cache_.value();
        }
        FormulaInterface::Value result = formula_->Evaluate(sheet_);
        if (const double* result_ptr = std::get_if<double>(&result)) {
            cache_ = *result_ptr;
            return *result_ptr;
        }
        else {
            cache_ = std::get<FormulaError>(result);
            return std::get<FormulaError>(result);
        }
    }

    void ClearCache() {
        cache_.reset();
    }

    bool EmptyCache() {
        return !cache_.has_value();
    }

    std::string GetText() const override {
        return FORMULA_SIGN + formula_->GetExpression();
    }

    std::vector<Position> GetReferencedCells() {
        return formula_->GetReferencedCells();
    }

    bool IsFormula() const override { return true; }

private:
    SheetInterface& sheet_;
    std::unique_ptr<FormulaInterface> formula_;
    mutable std::optional<CellInterface::Value> cache_;
};

void Cell::CheckCyclicity(Cell* cell_ptr) {
    using namespace std::literals;
    for (const Position& pos : cell_ptr->GetReferencedCells()) {
        if (sheet_.GetCell(pos) == nullptr) {
            continue;
        }
        if (disabling_cache_.count(sheet_.GetConcreteCell(pos)) > 0) {
            throw CircularDependencyException(""s);
        }
        if (hold_cells_ptr_.count(sheet_.GetConcreteCell(pos)) < 1) {
            disabling_cache_.insert(sheet_.GetConcreteCell(pos));
            if (sheet_.GetConcreteCell(pos)->IsReferenced()) {
                CheckCyclicity(sheet_.GetConcreteCell(pos));
            }
            disabling_cache_.erase(sheet_.GetConcreteCell(pos));
            hold_cells_ptr_.insert(sheet_.GetConcreteCell(pos));
        }
    }
}

void Cell::Set(std::string text) {

    if (text[0] == '=' && text.size() > 1) {
        impl_ = std::make_unique<FormulaImpl>((SheetInterface&)sheet_, text.substr(1));
        referenced_cell_ = static_cast<FormulaImpl*>(impl_.get())->GetReferencedCells();
        disabling_cache_.clear();
        hold_cells_ptr_.clear();
        CheckCyclicity(this);
    }
    else {
        if (text.empty()) {
            impl_ = std::make_unique<EmptyImpl>();
        }
        else {
            impl_ = std::make_unique<TextImpl>(text);
        }
    }
}

void Cell::ClearCache() {
    if (impl_->IsFormula()) {
        (dynamic_cast<FormulaImpl*>(impl_.get()))->ClearCache();
    }
}

bool Cell::IsUpReferenced() const {
    return !up_referenced_cell_.empty();
}

void Cell::CopyUpReferenceFromCell(Cell& other) {
    up_referenced_cell_ = other.up_referenced_cell_;
}

std::unordered_set<Cell*> Cell::GetUpReferenceCells() {
    return up_referenced_cell_;
}

void Cell::InsertCellPtrToUpReferencedList(Cell* cell_ptr) {
    up_referenced_cell_.emplace(std::move(cell_ptr));
}

std::vector<Position> Cell::GetReferencedCells() const { 
    return referenced_cell_;
}

bool Cell::IsReferenced() const { 
    return !referenced_cell_.empty();
}

Cell::Value Cell::GetValue() const {
    return impl_.get()->GetValue();
}
std::string Cell::GetText() const {
    return impl_.get()->GetText();
}