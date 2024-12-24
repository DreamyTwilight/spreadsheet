#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <unordered_set>

class Sheet;

class Cell : public CellInterface {
public:
    Cell(Sheet& sheet);
    ~Cell();

    void Set(std::string text);

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    bool IsReferenced() const;
    bool IsUpReferenced() const;
    void CopyUpReferenceFromCell(Cell& other);
    void InsertCellPtrToUpReferencedList(Cell* cell_ptr);
    std::unordered_set<Cell*> GetUpReferenceCells();
    void ClearCache();

private:
    class Impl;
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;

    Sheet& sheet_;
    std::unique_ptr<Impl> impl_;
    std::vector<Position> referenced_cell_;
    std::unordered_set<Cell*> up_referenced_cell_;
    std::unordered_set<Cell*> disabling_cache_;
    std::unordered_set<Cell*> hold_cells_ptr_;

    void CheckCyclicity(Cell* cell_ptr);
};
