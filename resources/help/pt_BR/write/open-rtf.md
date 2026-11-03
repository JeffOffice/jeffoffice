# Abrir arquivos RTF
<!-- keywords: rtf, rich text, rich text format, formato rich text, abrir rtf, .rtf, documentos antigos, importar, abrir, converter, texto formatado -->

**RTF** (Rich Text Format) é um tipo de arquivo de documento que quase todos os processadores de texto conseguem ler e gravar. O JeffWrite abre arquivos **.rtf** e mantém o texto e a aparência deles.

## Abrir um arquivo RTF

Pressione **Ctrl+O** ou clique em **Arquivo > Abrir** e clique em **Procurar**. Escolha **Rich Text Format (*.rtf)** na lista de tipos de arquivo para ver somente esses arquivos, ou selecione o arquivo na lista de todos os documentos. Você também pode arrastar um arquivo .rtf para uma janela do JeffWrite ou informar o nome dele ao iniciar o JeffWrite pela linha de comando.

O JeffWrite reconhece um arquivo RTF pelo conteúdo, e não pelo nome. Assim, um arquivo com outra extensão também é aberto, desde que seja realmente um arquivo RTF.

## O que é importado

O JeffWrite lê estas partes de um arquivo RTF:

- Texto em qualquer idioma, com fontes, tamanhos, cores, negrito, itálico, sublinhado, tachado, sobrescrito, subscrito e realce.
- Configurações de parágrafo: alinhamento, recuos, espaçamento, paradas de tabulação, bordas, sombreamento e as configurações de manter com o próximo e manter linhas juntas.
- Estilos, listas numeradas e listas com marcadores.
- Tabelas, incluindo tabelas dentro de tabelas, bordas, sombreamento, larguras de coluna e células mescladas.
- Tamanho da página, margens, colunas, orientação da página, seções, cabeçalhos e rodapés.
- Notas de rodapé, notas de fim, campos de número de página, outros campos e indicadores.
- Alterações controladas: texto adicionado, texto excluído, quebras de parágrafo adicionadas ou excluídas e alterações de formatação. Uma **alteração controlada** é uma edição marcada por um revisor. Consulte [Controlar alterações e aceitá-las ou rejeitá-las](track-changes).
- Imagens nos formatos PNG, JPEG, EMF e WMF, além de quebras de página e quebras de coluna.

Se alguma parte do arquivo não puder ser lida, o JeffWrite informa o que foi omitido. Alguns arquivos RTF contêm itens que um documento do JeffWrite ainda não comporta, como desenhos ou equações. Esses itens são omitidos, e o texto ao redor deles é mantido.

## Salvar as alterações

O JeffWrite não grava arquivos .rtf. Quando você salva um documento que veio de um arquivo .rtf, o JeffWrite pede um novo nome e o salva como arquivo .docx. O arquivo .rtf permanece como estava.

## Relacionados

- [Abrir, salvar, imprimir e exportar](open-save)
- [Bem-vindo ao JeffWrite](start)
