# Salvar um bloco e inseri-lo novamente
<!-- keywords: blocos salvos, texto padrão, assinatura, papel timbrado, atalho de texto, atalhos de texto, reutilizar texto, salvar seleção, inserir novamente, F3, biblioteca -->

Um **bloco** é uma parte de um documento que você salva para poder inseri-la novamente: uma assinatura, um parágrafo que você usa em todas as cartas, um papel timbrado ou uma imagem com legenda. Um bloco mantém seu texto junto com as fontes, cores, estilos, marcadores e números, imagens, links, campos e tabelas (linhas, colunas, larguras, bordas, sombreamento, células mescladas, uma linha de cabeçalho que se repete e o estilo da tabela). Ele não mantém comentários, notas de rodapé, notas de fim nem quebras de seção (veja Informações úteis).

## Salvar um bloco

1. Selecione o que você deseja salvar.
2. Escolha **Inserir > Blocos Salvos > Salvar Seleção como Bloco**.
3. Digite um **Nome**. Você também pode adicionar uma **Categoria** e uma **Descrição**, que ajudam a encontrar o bloco depois.
4. Em **Galeria**, escolha **Blocos Salvos** (um bloco que você seleciona em uma galeria) ou **Atalhos de Texto** (um bloco que você insere digitando o seu nome).
5. Em **Inserir como**, escolha como o bloco será inserido em um documento: **Como está, no cursor**, **Em um parágrafo próprio** ou **Em uma página própria**.
6. Escolha **Salvar**.

Se já existir um bloco salvo com esse nome, o JeffWrite pergunta se você deseja substituí-lo.

## Inserir um bloco

1. Posicione o cursor onde o bloco deve ficar. Se você selecionar palavras antes, o bloco ocupará o lugar delas.
2. Escolha **Inserir > Blocos Salvos** e clique no bloco. Digite na caixa de pesquisa para encontrar um bloco pelo nome, categoria ou descrição.

Inserir um bloco pode ser desfeito em uma única etapa. Um estilo usado pelo bloco que o seu documento não tem é adicionado; um estilo que o documento já tem com o mesmo nome nunca é alterado. Um campo do bloco, como o número da página, é atualizado para o documento em que está. O primeiro e o último parágrafo de um bloco assumem a aparência dos parágrafos que eles unem, como ocorre ao colar.

## Atalhos de Texto

Um Atalho de Texto é um bloco com um nome curto. Digite o nome e pressione **F3** (ou escolha **Inserir > Blocos Salvos > Expandir Atalho de Texto**), e o nome será substituído pelo bloco. O nome precisa começar uma palavra: "ass" funciona depois de um espaço, mas não dentro de "passagem".

## Gerenciar blocos salvos

Escolha **Inserir > Blocos Salvos > Gerenciar Blocos Salvos**. Você pode ver cada bloco, alterar seu nome, categoria, descrição ou a forma como é inserido com **Editar Propriedades**, e **Excluir** o bloco. **Desfazer Exclusão** restaura o que você excluiu até fechar a caixa. **Exportar** salva seus blocos em um arquivo, e **Importar** adiciona os blocos de um desses arquivos; um bloco cujo nome você já tem é mantido como está.

## Informações úteis

- Seus blocos salvos ficam em um único arquivo neste computador, somente para a sua conta: não estão nos seus documentos e não são enviados a lugar nenhum. O arquivo é `library.jblocks`, na pasta `Saved Blocks` dentro da sua pasta de dados: `AppData\Local\JeffOffice` no Windows, `Library/Application Support/JeffOffice` no Mac e `.local/share/JeffOffice` no Linux. Todos os programas JeffOffice do seu computador compartilham esse arquivo, e duas janelas podem adicionar blocos ao mesmo tempo sem que um perca os blocos do outro.
- Um bloco pode ter até 8 megabytes, e a biblioteca comporta até 5.000 blocos.
- Uma tabela é salva como tabela quando a seleção contém a tabela inteira. Se a seleção contiver apenas parte de uma tabela, essa parte é salva como texto, com um parágrafo para cada célula, e o JeffWrite avisa você antes de salvar.
- Um indicador em um bloco recebe um nome próprio quando você insere o bloco: se o seu documento já tiver um indicador com esse nome, o novo recebe "_2", "_3" e assim por diante depois do nome.
- Comentários são deixados de fora. Notas de rodapé e notas de fim são deixadas de fora. Quebras de seção não são mantidas. Formas e caixas de texto são mantidas e voltam junto com o bloco.
- As alterações controladas são aceitas ao salvar o bloco: as palavras inseridas permanecem, e as palavras excluídas não entram no bloco, de modo que um bloco nunca contém palavras controladas de outra pessoa.
- Um documento nunca adiciona blocos à sua biblioteca. Somente você faz isso.
