#include "pmfColumn.h"


PmfColumn::PmfColumn(const COL_SPEC * colSpec)
{
    _colName = colSpec->ColName;
    _colType = colSpec->ColType;
    _length = colSpec->Length;
    _scale = colSpec->Scale;
    _nullable = colSpec->Nullable;
    _default = colSpec->Default;
    _logged = colSpec->Logged;
    _compact = colSpec->Compact;
    _identity = colSpec->Identity;
    _generated = colSpec->Generated;
    _misc = colSpec->Misc;
}

PmfColumn::~PmfColumn()
{

}



