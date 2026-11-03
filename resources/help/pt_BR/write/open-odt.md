# Abrir e salvar arquivos OpenDocument
<!-- keywords: odt, fodt, ott, opendocument, open document, documento aberto, abrir, salvar, salvar como, formato, tipo de arquivo, outros programas, trocar documentos, compartilhar, compatibilidade, alterações controladas, controlar alterações, omitido, ignorado, modelo -->

O JeffWrite abre e salva texto OpenDocument, o tipo de arquivo cujo nome termina em **.odt**. Muitos programas gratuitos leem e gravam esse formato, por isso ele é uma boa opção para trocar documentos com quem não usa o JeffWrite.

## Abrir um arquivo OpenDocument

Pressione **Ctrl+O** ou clique em **Arquivo > Abrir** e escolha um arquivo:

- **.odt** é um documento.
- **.ott** é um modelo. Ele é aberto como um novo documento criado a partir dele, de modo que o modelo permanece inalterado.
- **.fodt** é o mesmo tipo de documento, guardado em um único arquivo XML simples. O JeffWrite não grava arquivos .fodt: ao usar **Salvar** em um deles, ele pede um novo nome e salva o arquivo como .odt.

O JeffWrite identifica o tipo de arquivo pelo conteúdo, e não pelo nome, por isso um arquivo com a extensão errada também é aberto. Você também pode informar o nome de um arquivo ao iniciar o JeffWrite ou arrastá-lo do gerenciador de arquivos para o programa, como faz com um arquivo .docx.

O JeffWrite lê o texto e sua formatação, títulos, estilos e os estilos em que eles se baseiam, listas com marcadores e numeradas, tabelas (com células mescladas, largura das colunas, bordas, sombreamento e tabelas dentro de tabelas), seções e colunas, o tamanho, as margens e a direção da página, cabeçalhos e rodapés (da primeira página, das páginas da esquerda e da direita), notas de rodapé e notas de fim, campos (números de página, contagem de páginas, título, autor, nome do arquivo, referências cruzadas e numeração sequencial), indicadores, links, sumário, imagens, caixas de texto, paradas de tabulação, comentários e alterações controladas, que aparecem como marcas que você pode aceitar ou rejeitar.

Alguns itens o JeffWrite ainda não consegue mostrar: formas desenhadas que não contêm texto, fórmulas, gráficos e outros objetos inseridos no texto. Se um arquivo contiver algum deles, o JeffWrite informa quais ao abri-lo. Nesse caso, ele não salva por cima desse arquivo: **Salvar** pede um novo nome, para que o original mantenha o que a cópia perdeu. Documentos protegidos por senha não são abertos. Abra-os no programa em que foram criados e salve uma cópia sem a senha.

## Salvar um arquivo OpenDocument

Clique em **Arquivo > Salvar Como** e escolha **Texto OpenDocument (*.odt)** na lista de tipos de arquivo. Depois disso, **Ctrl+S** salva novamente no mesmo formato.

O arquivo contém tudo o que foi listado acima, com os comentários e as alterações controladas: texto adicionado e excluído, formatação alterada e quebras de parágrafo adicionadas ou excluídas. Para alguns itens o formato não tem lugar, e o JeffWrite informa quais são depois que você salva: eles são salvos como estão no momento ou omitidos. São eles: alterações controladas em tabelas, colunas e configuração da página, e alterações controladas em cabeçalhos, rodapés e notas (salvas como estão no momento); campos de formulário (salvos como o texto que exibem); equações (salvas como texto sem formatação); e desenhos, formas, gráficos e objetos inseridos que não podem ser mantidos. Se você salvar novamente após uma alteração que faça algo novo ser omitido, o JeffWrite também avisa sobre isso, sem repetir o que já informou.

Outros programas que leem OpenDocument abrem o arquivo sem pedir para repará-lo.

## Relacionados

- [Abrir, salvar, imprimir e exportar](open-save)
- [Controlar alterações e aceitá-las ou rejeitá-las](track-changes)
- [Bem-vindo ao JeffWrite](start)
