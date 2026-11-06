# Contar palavras, verificar e pesquisar palavras
<!-- keywords: células mescladas, texto do link, contar palavras, acessibilidade, verificar acessibilidade, texto alternativo, linha de cabeçalho, leitor de tela, contraste, dicionário de sinônimos, sinônimo, sinônimos, traduzir, tradução, libretranslate, contagem de palavras, contar palavras, caracteres, parágrafos, páginas, seleção, notas de rodapé, notas de fim, idioma, idioma de revisão, definir idioma, não verificar ortografia, idioma da ortografia, barra de status -->

Essas ferramentas estão na guia **Revisão**.

## Contar as palavras

1. Clique em **Contar Palavras** no grupo **Revisão de Texto**.
2. A caixa mostra as páginas, as palavras, os caracteres sem espaços, os caracteres com espaços e os parágrafos.

Se você selecionar um texto antes, a caixa conta somente esse texto. A barra de status, na parte inferior da janela, também mostra isso, como "12 de 340 palavras".

Marque **Incluir notas de rodapé e notas de fim** para contá-las também. O texto oculto nunca é contado. Também não é contado o texto que uma alteração controlada excluiu.

## Definir o idioma do texto

O JeffWrite verifica a ortografia com o dicionário do idioma em que o texto está. Se você marcar uma citação em francês como francês, as palavras dela serão verificadas em francês.

1. Selecione o texto.
2. No grupo **Idioma**, clique em **Definir Idioma de Revisão de Texto**. Você também pode clicar no nome do idioma na barra de status.
3. Escolha um idioma e clique em **OK**.

O idioma é gravado no arquivo, então outros programas também o reconhecem. A barra de status mostra o idioma do texto onde está o cursor.

## Não verificar alguns textos

Alguns textos não devem ser verificados: um código, um nome ou uma linha em um idioma inventado. Selecione-o e clique em **Não Verificar Ortografia** no grupo **Idioma**. As linhas vermelhas onduladas desaparecem. Clique de novo para verificar o texto outra vez.

## Pesquisar uma palavra no dicionário de sinônimos

Um dicionário de sinônimos lista palavras de significado quase igual.

1. Clique em uma palavra ou selecione-a.
2. Clique em **Dicionário de Sinônimos** no grupo **Revisão de Texto** ou pressione **Shift+F7**.
3. Escolha uma palavra na lista e clique em **Substituir**. Para pesquisar uma das palavras da lista, clique em **Pesquisar**. Você também pode digitar outra palavra na caixa e pressionar **Enter**.

A lista vem de um arquivo do seu computador. Nenhuma conexão com a internet é usada.

## Traduzir texto

1. Selecione o texto. Se nada estiver selecionado, todo o documento será usado.
2. Clique em **Traduzir** no grupo **Idioma**.
3. Escolha o idioma para o qual deseja traduzir.
4. Clique em **OK**.

Em seguida, seu navegador da web abre um serviço de tradução chamado LibreTranslate, com o seu texto nele. Nada é enviado até que você clique em **OK**. Você pode alterar o endereço do serviço na caixa, por exemplo, para um que você mesmo mantenha. Somente endereços da web (http ou https) são usados. São enviadas no máximo 5.000 letras.

## Verificar a acessibilidade

Algumas pessoas leem com um leitor de tela, que lê o documento em voz alta. Outras têm dificuldade para ver textos claros. A verificação de acessibilidade encontra o que torna um documento difícil para elas.

1. Clique em **Verificar Acessibilidade** no grupo **Revisão de Texto**.
2. Uma lista se abre à direita. Cada linha diz o que está errado. Clique em uma linha para ir até o lugar.
3. Corrija o problema. Clique em **Verificar Novamente**.

A verificação procura estes problemas:

- **Uma imagem sem texto alternativo.** O texto alternativo é uma descrição curta de uma imagem. Para adicioná-lo, clique com o botão direito na imagem e escolha o texto alternativo.
- **Uma tabela sem linha de cabeçalho.** A linha de cabeçalho contém os nomes das colunas.
- **Uma célula de tabela em branco.**
- **Um título que pula um nível**, como Título 3 logo depois de Título 1.
- **Um texto difícil de ler**, porque a cor dele é muito parecida com a cor do fundo.
- **Uma imagem flutuando na página**, em vez de estar alinhada com o texto.
- **Uma tabela com células mescladas.** Uma célula mesclada é uma única célula que ocupa duas ou mais colunas ou linhas. Um leitor de tela pode perder a referência em uma tabela assim. Isso é indicado uma vez para cada tabela.
- **Um link cujo texto não diz nada sobre para onde ele leva**, como "clique aqui" ou "leia mais", ou um endereço da web mostrado como está. Quem ouve a lista de links de uma página precisa de palavras que identifiquem a página.

Você também pode iniciar a verificação em **Arquivo > Informações**: clique em **Verificar Acessibilidade**.
