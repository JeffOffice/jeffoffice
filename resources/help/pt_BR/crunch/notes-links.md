# Notas e links
<!-- keywords: nota, notas, anotação, anotações, comentário, comentários, passar o ponteiro, marca vermelha, canto, nova anotação, editar anotação, excluir anotação, mostrar anotação, ocultar anotação, mostrar todas as anotações, link, links, hiperlink, arquivos recentes, endereço, página da web, email, e-mail, local nesta pasta de trabalho, Ctrl+K, Ctrl+clique, dica de tela, abrir link, remover link, conversa, estatísticas da pasta de trabalho, contagem, células, fórmulas, gráficos, tabelas dinâmicas, acessibilidade, texto alternativo, células mescladas -->

## Notas

Uma **nota** é um pequeno texto guardado junto de uma célula. Uma célula que tem uma nota mostra uma pequena marca vermelha no canto superior direito. Ao apontar para a célula, a nota aparece ao lado dela; ao tirar o ponteiro, ela desaparece.

Para colocar uma nota em uma célula, selecione-a e use **Revisão > Nova Anotação** (ou pressione **Shift+F2**). Digite o texto e clique em **OK**. Em uma célula que já tem anotação, o mesmo comando aparece como **Editar Anotação**. Apagar o texto remove a anotação.

- **Excluir Anotação** remove as anotações das células selecionadas.
- **Mostrar ou Ocultar Anotação** mantém a anotação das células selecionadas visível na planilha o tempo todo, em uma caixa ao lado de cada célula, ou volta a escondê-la.
- **Mostrar Todas as Anotações** faz o mesmo para todas as anotações da planilha; o mesmo botão passa então a se chamar **Ocultar Todas as Anotações**.
- **Anotação Anterior** e **Próxima Anotação** levam de uma célula com anotação para a seguinte.

Cada um desses comandos pode ser desfeito em uma etapa. Uma anotação acompanha a sua célula quando linhas ou colunas são inseridas ou excluídas e quando as células são classificadas. **Limpar Tudo** remove as anotações das células que limpa.

Uma pasta de trabalho pode conter uma conversa de comentários, com respostas. O JeffCrunch mostra a conversa inteira como uma anotação e a mantém ao salvar, mas não a altera: a anotação é somente leitura.

As anotações podem ser salvas em uma pasta de trabalho .xlsx. Uma pasta de trabalho aberta a partir de um tipo de arquivo mais antigo, ou de um arquivo de texto, mantém as suas anotações somente depois que você a salvar como pasta de trabalho .xlsx.

## Links

Um **link** leva você a algum lugar quando você o segue: a uma página da web, a um arquivo, a um endereço de email ou a um local nesta pasta de trabalho (uma célula, um conjunto de células ou um nome).

Para colocar um link em uma célula, selecione-a e use **Inserir > Link** (ou pressione **Ctrl+K**). Escolha o destino, digite o endereço ou o local (ou escolha um dos arquivos abertos recentemente em **Ou um arquivo recente**) e, se quiser, o texto que a célula deve mostrar e uma **dica de tela**. Em uma célula que já tem link, o mesmo comando aparece como **Editar Link**. **Remover Link** tira os links das células selecionadas.

Aponte para uma célula com link para ver para onde ele leva. Para segui-lo, mantenha **Ctrl** pressionada e clique na célula, ou selecione a célula e use **Abrir Link** no menu do botão direito. Um clique simples apenas seleciona a célula.

O JeffCrunch nunca abre nada por conta própria. Uma página da web ou um endereço de email é aberto no seu programa habitual quando você segue o link. Um link para um arquivo, ou para outro tipo de endereço, pede a sua confirmação antes. Um link que executaria um script nunca é aberto.

Um link acompanha a sua célula quando linhas ou colunas são inseridas ou excluídas e quando as células são classificadas, e é salvo junto com a pasta de trabalho.

## Estatísticas da Pasta de Trabalho

**Estatísticas da Pasta de Trabalho** (guia **Revisão**) conta o que a pasta de trabalho contém: quantas **Planilhas** ela tem, quantas **Células com dados** (células que contêm um número, um texto ou uma fórmula), quantas **Fórmulas**, quantos **Gráficos** e quantas **Tabelas dinâmicas**. Planilhas excluídas não são contadas. Os mesmos números aparecem na página **Informações** da guia Arquivo.

## Verificar Acessibilidade

**Verificar Acessibilidade** (guia **Revisão**) abre uma pequena janela que lista o que na pasta de trabalho é difícil de usar para alguns leitores, ou para um leitor de tela (um programa que lê a tela em voz alta). Cada linha começa com **Erro**, **Aviso** ou **Dica**. Clique em uma linha para ir até a célula.

Ele procura:

- Um gráfico ou uma imagem sem **texto alternativo** (as palavras que descrevem o que ela mostra). Isto é um erro.
- Uma tabela sem linha de cabeçalho, de modo que suas colunas não têm nomes. Isto é um erro.
- **Células mescladas**, que são difíceis de percorrer com o teclado. Isto é um aviso.
- Texto difícil de ler sobre a cor de fundo, como texto cinza-claro em branco. O JeffCrunch mede o quanto o texto se destaca do preenchimento. Textos pequenos precisam de uma proporção de pelo menos 4,5 para 1, e textos grandes (18 pontos, ou negrito de 14 pontos) precisam de 3 para 1. Ele indica a primeira célula de cada combinação de fonte e preenchimento. Isto é um aviso.
- Uma planilha que ainda tem o nome que recebeu, como Planilha1. Isto é uma dica.

Pressione **Verificar Novamente** depois de fazer alterações.
