#include "mscoff_generator.h"

MSCOFFGenerator::MSCOFFGenerator(): PatternGenerator() { }
std::string MSCOFFGenerator::name() const { return "MS Common Object File Format (COFF)"; }
std::string MSCOFFGenerator::assembler() const { return m_loader->assembler(); }

bool MSCOFFGenerator::test(const std::string &infile)
{
    std::unique_ptr<REDasm::MemoryBuffer> mb(REDasm::MemoryBuffer::fromFile(infile));
    REDasm::LoadRequest lr(infile, mb.get());
    return REDasm::MSCOFFLoader::test(lr, static_cast<REDasm::ImageArchiveHeader*>(lr.view));
}

void MSCOFFGenerator::generate(const std::string &infile)
{
    REDasm::LoadRequest lr(infile, REDasm::MemoryBuffer::fromFile(infile));
    m_loader = std::unique_ptr<REDasm::MSCOFFLoader>(static_cast<REDasm::MSCOFFLoader*>(REDasm::mscoff_plugin_loader_init(lr)));
    m_loader->load();

    const auto& functions = m_loader->functions();

    for(auto it = functions.begin(); it != functions.end(); it++)
    {
        REDasm::BufferView view = m_loader->view(static_cast<address_t>(it->start));

        if(view.eob())
            continue;

        std::string pattern = REDasm::hexstring(view, it->size);
        const auto* relocations = m_loader->relocations(it->section);

        if(relocations)
        {
            for(size_t i = 0; i < relocations->size(); i++)
            {
                const REDasm::RelocationItem& reloc = relocations->at(i);

                if((it->start < reloc.offset) || ((reloc.offset + reloc.size) >= (it->start + it->size)))
                    continue;

                this->wildcard(pattern, reloc.offset, reloc.size);
            }
        }

        this->push({ REDasm::SymbolTypes::Function, it->name, pattern });
    }
}
