# Anotações e links
<!-- keywords: thesaurus, sinônimos, lista de palavras, anotação, anotações, comentário, comentários, passar o ponteiro, marca vermelha, canto, nova anotação, editar anotação, excluir anotação, mostrar anotação, ocultar anotação, mostrar todas as anotações, link, links, hiperlink, endereço, página da web, email, lugar nesta pasta de trabalho, Ctrl+K, Ctrl+clique, dica de tela, abrir link, remover link, conversa, estatísticas da pasta de trabalho, contar, células, fórmulas, gráficos, tabelas dinâmicas, acessibilidade, texto alternativo, células mescladas, arquivos recentes -->

## Anotações

Uma **anotação** é um pequeno texto guardado junto de uma célula. Uma célula que tem anotação mostra uma pequena marca vermelha no canto superior direito. Ao passar o ponteiro sobre a célula, a anotação aparece ao lado dela; ao tirar o ponteiro, ela desaparece.

Para colocar uma anotação em uma célula, selecione-a e use **Revisão > Nova Anotação** (ou pressione **Shift+F2**). Digite o texto e clique em **OK**. Em uma célula que já tem anotação, o mesmo comando aparece como **Editar Anotação**. Apagar o texto remove a anotação.

- **Excluir Anotação** remove as anotações das células selecionadas.
- **Mostrar ou Ocultar Anotação** mantém a anotação das células selecionadas visível na planilha o tempo todo, em uma caixa ao lado de cada célula, ou volta a ocultá-la.
- **Mostrar Todas as Anotações** faz o mesmo para todas as anotações da planilha; o mesmo botão passa a ser **Ocultar Todas as Anotações**.
- **Anotação Anterior** e **Próxima Anotação** levam de uma célula com anotação para a seguinte.

Cada uma dessas ações pode ser desfeita em um único passo. Uma anotação acompanha a sua célula quando linhas ou colunas são inseridas ou excluídas e quando as células são classificadas. **Limpar Tudo** remove as anotações das células que limpa.

Uma pasta de trabalho pode conter uma conversa de comentários, com respostas. O JeffCrunch mostra a conversa inteira como uma anotação e a mantém ao salvar, mas não a altera: a anotação é somente leitura.

As anotações podem ser salvas em uma pasta de trabalho .xlsx. Uma pasta de trabalho aberta de um tipo de arquivo mais antigo, ou de um arquivo de texto, mantém as suas anotações somente depois que você a salvar como pasta de trabalho .xlsx.

## Links

Um **link** leva você a outro lugar quando você o segue: a uma página da web, a um arquivo, a um endereço de email ou a um lugar nesta pasta de trabalho (uma célula, células ou um nome).

Para colocar um link em uma célula, selecione-a e use **Inserir > Link** (ou pressione **Ctrl+K**). Escolha para onde ele leva, digite o endereço ou o lugar (ou escolha um dos arquivos abertos recentemente em **Ou um arquivo recente**) e, se quiser, o texto que a célula deve mostrar e uma **dica de tela**. Em uma célula que já tem link, o mesmo comando aparece como **Editar Link**. **Remover Link** tira os links das células selecionadas.

Passe o ponteiro sobre uma célula com link para ver para onde ele leva. Para segui-lo, mantenha **Ctrl** pressionada e clique na célula, ou selecione a célula e use **Abrir Link** no menu do botão direito. Um clique simples apenas seleciona a célula.

O JeffCrunch nunca abre nada por conta própria. Uma página da web ou um endereço de email abre no seu programa habitual quando você segue o link. Um link para um arquivo, ou para outro tipo de endereço, pede a sua confirmação antes. Um link que executaria um script nunca é aberto.

Um link acompanha a sua célula quando linhas ou colunas são inseridas ou excluídas e quando as células são classificadas, e é salvo junto com a pasta de trabalho.

## Estatísticas da Pasta de Trabalho

**Estatísticas da Pasta de Trabalho** (guia **Revisão**) conta o que a pasta de trabalho contém: quantas **Planilhas** ela tem, quantas **Células com dados** (células que contêm um número, um texto ou uma fórmula), quantas **Fórmulas**, quantos **Gráficos** e quantas **Tabelas dinâmicas**. As planilhas excluídas não são contadas. Os mesmos números aparecem na página **Informações** da guia Arquivo.

## Dicionário de Sinônimos

**Dicionário de Sinônimos** (guia **Revisão**, ou **Shift+F7**) encontra palavras que significam quase o mesmo que uma palavra em uma célula. Ele procura a primeira palavra do texto da célula em que você está. Digite outra palavra em **Pesquisar** e pressione **Enter** para pesquisar essa. Escolha uma palavra na lista e clique em **Substituir**, ou clique duas vezes nela. A palavra na célula é substituída, e uma primeira letra maiúscula continua maiúscula. **Pesquisar** procura a palavra que você escolheu, para acompanhar uma cadeia de significados. A lista de palavras funciona em inglês.

## Verificar Acessibilidade

**Verificar Acessibilidade** (guia **Revisão**) abre uma pequena janela que lista o que na pasta de trabalho é difícil de usar para alguns leitores, ou para um leitor de tela (um programa que lê a tela em voz alta). Cada linha começa com **Erro**, **Aviso** ou **Dica**. Clique em uma linha para ir até a célula.

Ele procura:

- Um gráfico ou imagem sem **texto alternativo** (as palavras que descrevem o que ele mostra). Isso é um erro.
- Uma tabela sem linha de cabeçalho, de modo que as colunas não têm nomes. Isso é um erro.
- **Células mescladas**, que são difíceis de percorrer com o teclado. Isso é um aviso.
- Texto difícil de ler sobre a cor de fundo, como texto cinza-claro em branco. O JeffCrunch mede o quanto o texto se destaca do preenchimento. Texto pequeno precisa de uma proporção de pelo menos 4,5 para 1, e texto grande (18 pontos, ou negrito de 14 pontos) precisa de 3 para 1. Ele indica a primeira célula de cada combinação de fonte e preenchimento. Isso é um aviso.
- Uma planilha que ainda tem o nome que recebeu, como Planilha1. Isso é uma dica.

Clique em **Verificar Novamente** depois de fazer alterações.
