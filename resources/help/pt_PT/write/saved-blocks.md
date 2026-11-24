# Guardar um bloco e voltar a inseri-lo

<!-- keywords: blocos guardados, texto-modelo, assinatura, papel timbrado, atalho de texto, atalhos de texto, reutilizar texto, guardar seleção, inserir novamente, F3, biblioteca -->

Um **bloco** é uma parte de um documento que guardou para a poder inserir novamente: uma assinatura, um parágrafo que usa em todas as cartas, um papel timbrado ou uma imagem com legenda. Um bloco mantém o seu texto com os respetivos tipos de letra, cores, estilos, marcas e números, imagens, ligações, campos e tabelas (linhas, colunas, larguras, limites, sombreado, células unidas, uma linha de cabeçalho que se repete e o estilo da tabela). Não mantém comentários, notas de rodapé, notas de fim nem quebras de secção (ver Good to know).

## Guardar um bloco

1. Selecione o que pretende guardar.
2. Escolha **Inserir > Blocos Guardados > Guardar Seleção como Bloco**.
3. Escreva um **Nome**. Pode também acrescentar uma **Categoria** e uma **Descrição**, que ajudam a encontrar o bloco mais tarde.
4. Em **Galeria**, escolha **Blocos Guardados** (um bloco que escolhe numa galeria) ou **Atalhos de Texto** (um bloco que é inserido ao escrever o seu nome).
5. Em **Inserir como**, escolha a forma como o bloco entra num documento: **Tal como está, no cursor**, **Num parágrafo próprio** ou **Numa página própria**.
6. Escolha **Guardar**.

Se já existir um bloco guardado com esse nome, o JeffWrite pergunta se deve substituí-lo.

## Inserir um bloco

1. Coloque o cursor onde o bloco deve ficar. Se selecionar primeiro algumas palavras, o bloco ocupa o seu lugar.
2. Escolha **Inserir > Blocos Guardados** e, em seguida, clique no bloco. Escreva na caixa de pesquisa para encontrar um bloco pelo nome, categoria ou descrição.

Inserir um bloco é um passo que pode ser anulado. Um estilo usado pelo bloco que o seu documento não tenha é acrescentado; um estilo que o documento já tenha com o mesmo nome nunca é alterado. Um campo do bloco, como o número de página, é atualizado para o documento em que está. O primeiro e o último parágrafo de um bloco assumem o aspeto dos parágrafos que os rodeiam, tal como acontece ao colar.

## Atalhos de Texto

Um Atalho de Texto é um bloco com um nome curto. Escreva o nome e prima **F3** (ou escolha **Inserir > Blocos Guardados > Expandir Atalho de Texto**), e o nome é substituído pelo bloco. O nome tem de começar uma palavra: "ass" funciona depois de um espaço, mas não dentro de "assinatura".

## Gerir blocos guardados

Escolha **Inserir > Blocos Guardados > Gerir Blocos Guardados**. Pode ver cada bloco, alterar o seu nome, categoria, descrição ou a forma como é inserido com **Editar Propriedades**, e **Eliminar** o bloco. **Anular Eliminação** repõe o que eliminou até fechar a caixa. **Exportar** guarda os seus blocos num ficheiro, e **Importar** acrescenta os blocos de um desses ficheiros; um bloco cujo nome já existe é mantido como está.

## Good to know

- Os seus blocos guardados ficam num único ficheiro neste computador, apenas para a sua conta: não estão nos seus documentos e não são enviados para lado nenhum. O ficheiro é `library.jblocks`, na pasta `Saved Blocks` dentro da pasta de dados: `AppData\Local\JeffOffice` no Windows, `Library/Application Support/JeffOffice` no Mac e `.local/share/JeffOffice` no Linux. Todos os programas JeffOffice do seu computador partilham este ficheiro, e duas janelas podem acrescentar blocos ao mesmo tempo sem que se percam os blocos uma da outra.
- Um bloco pode ter até 8 megabytes, e a biblioteca pode conter até 5 000 blocos.
- Uma tabela é guardada como tabela quando a seleção inclui toda a tabela. Se a seleção incluir apenas parte de uma tabela, essa parte é guardada como texto, com um parágrafo por cada célula, e o JeffWrite avisa antes de guardar.
- Um marcador num bloco recebe um nome próprio ao ser inserido: se o seu documento já tiver um marcador com esse nome, o novo é designado com "_2", "_3" e assim sucessivamente.
- Os comentários são omitidos. As notas de rodapé e as notas de fim são omitidas. As quebras de secção não são mantidas. As formas e as caixas de texto são mantidas e voltam a ser inseridas com o bloco.
- As alterações controladas são aceites ao guardar o bloco: o texto inserido mantém-se, e o texto eliminado não fica no bloco, pelo que um bloco nunca contém texto controlado de outra pessoa.
- Um documento nunca acrescenta blocos à sua biblioteca. Só você o faz.
