# Notas e ligações

<!-- keywords: nota, notas, comentário, comentários, pairar, marca vermelha, canto, nova nota, editar nota, eliminar nota, mostrar nota, ocultar nota, mostrar todas as notas, ligação, ligações, hiperligação, ficheiros recentes, endereço, página web, correio eletrónico, local neste livro, Ctrl+K, Ctrl+clique, sugestão de ecrã, abrir ligação, remover ligação, conversa, estatísticas do livro, contar, células, fórmulas, gráficos, tabelas dinâmicas, acessibilidade, texto alternativo, células unidas -->

## Notas

Uma **nota** é um pequeno texto associado a uma célula. Uma célula que tenha uma nota mostra uma pequena marca vermelha no canto superior direito. Quando poisa o ponteiro na célula, a nota aparece ao lado dela; quando afasta o ponteiro, a nota desaparece.

Para colocar uma nota numa célula, selecione-a e utilize **Revisão > Nova Nota** (ou prima **Shift+F2**). Escreva o texto e clique em **OK**. Numa célula que já tenha uma nota, o mesmo comando aparece como **Editar Nota**. Se apagar o texto, a nota é removida.

- **Eliminar Nota** remove as notas das células selecionadas.
- **Mostrar ou Ocultar Nota** mantém a nota das células selecionadas sempre visível na folha, numa caixa ao lado de cada célula, ou volta a ocultá-la.
- **Mostrar Todas as Notas** faz o mesmo para todas as notas da folha; o mesmo botão passa então a ler-se **Ocultar Todas as Notas**.
- **Nota Anterior** e **Nota Seguinte** deslocam-se de uma célula com nota para a seguinte.

Cada uma destas ações pode ser anulada com um único passo. Uma nota acompanha a sua célula quando são inseridas ou eliminadas linhas ou colunas, e quando as células são ordenadas. **Limpar Tudo** remove as notas das células que limpa.

Um livro pode conter uma conversa de comentários, com respostas. O JeffCrunch mostra a conversa completa como uma nota e mantém-na ao guardar, mas não a altera: a nota é só de leitura.

As notas podem ser guardadas num livro .xlsx. Um livro aberto a partir de um tipo de ficheiro mais antigo, ou de um ficheiro de texto, só mantém as notas depois de o guardar como livro .xlsx.

## Ligações

Uma **ligação** leva-o a outro lugar quando a segue: a uma página web, a um ficheiro, a um endereço de correio eletrónico ou a um local neste livro (uma célula, um conjunto de células ou um nome).

Para colocar uma ligação numa célula, selecione-a e utilize **Inserir > Ligação** (ou prima **Ctrl+K**). Escolha para onde deve levar, escreva o endereço ou o local (ou escolha um dos ficheiros que abriu recentemente em **Ou um ficheiro recente**) e, se quiser, o texto que a célula deve mostrar e uma **sugestão de ecrã**. Numa célula que já tenha uma ligação, o mesmo comando aparece como **Editar Ligação**. **Remover Ligação** retira as ligações das células selecionadas.

Poise o ponteiro numa célula com ligação para ver para onde ela leva. Para a seguir, mantenha **Ctrl** premida e clique na célula, ou selecione a célula e utilize **Abrir Ligação** no menu de contexto (botão direito do rato). Um clique simples apenas seleciona a célula.

O JeffCrunch nunca abre nada por iniciativa própria. Uma página web ou um endereço de correio eletrónico abre-se no seu programa habitual quando segue a ligação. Uma ligação para um ficheiro, ou para outro tipo de endereço, pede-lhe confirmação primeiro. Uma ligação que executaria um script nunca é aberta.

Uma ligação acompanha a sua célula quando são inseridas ou eliminadas linhas ou colunas, e quando as células são ordenadas, e fica guardada com o livro.

## Estatísticas do Livro

**Estatísticas do Livro** (separador **Revisão**) conta o que o livro contém: quantas **Folhas** tem, quantas **Células com dados** (células que contêm um número, um texto ou uma fórmula), quantas **Fórmulas**, quantos **Gráficos** e quantas **Tabelas dinâmicas**. As folhas que eliminou não são contadas. Os mesmos números aparecem na página **Informações** do separador Ficheiro.

## Verificar Acessibilidade

**Verificar Acessibilidade** (separador **Revisão**) abre uma pequena janela que lista o que, no livro, é difícil de usar para alguns leitores, ou para um leitor de ecrã (um programa que lê o ecrã em voz alta). Cada linha começa por **Erro**, **Aviso** ou **Sugestão**. Clique numa linha para ir até à célula.

Procura:

- Um gráfico ou uma imagem sem **texto alternativo** (as palavras que descrevem o que mostra). Isto é um erro.
- Uma tabela sem linha de cabeçalho, pelo que as suas colunas não têm nomes. Isto é um erro.
- **Células unidas**, que são difíceis de percorrer com o teclado. Isto é um aviso.
- Texto difícil de ler sobre a cor de fundo, como texto cinzento-claro sobre branco. O JeffCrunch mede o contraste entre o texto e o preenchimento. O texto pequeno precisa de um rácio de pelo menos 4,5 para 1, e o texto grande (18 pontos, ou 14 pontos a negrito) precisa de 3 para 1. Indica a primeira célula de cada combinação de tipo de letra e preenchimento. Isto é um aviso.
- Uma folha que ainda tem o nome que lhe foi atribuído, como Folha1. Isto é uma sugestão.

Prima **Verificar Novamente** depois de fazer alterações.
