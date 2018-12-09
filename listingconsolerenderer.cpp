#include "listingconsolerenderer.h"
#include <iostream>

ListingConsoleRenderer::ListingConsoleRenderer(REDasm::DisassemblerAPI *disassembler): REDasm::ListingRenderer(disassembler) { }
void ListingConsoleRenderer::renderAll() { this->render(0, m_document->size()); }

void ListingConsoleRenderer::renderLine(const REDasm::RendererLine &rl)
{
    REDasm::ListingItem* item = m_document->itemAt(rl.documentindex);

    if(!item->is(REDasm::ListingItem::InstructionItem))
        return;

    std::cout << rl.text << std::endl;
}
