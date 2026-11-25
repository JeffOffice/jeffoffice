# Mala direta e mesclagem de catálogo
<!-- keywords: mala direta, mesclagem, destinatários, lista de endereços, etiquetas, cartas modelo, csv, xlsx, vcard, campo de mesclagem, bloco de endereço, catálogo, lista de produtos, mesclagem de email, visualizar -->

A mala direta preenche uma publicação com dados de uma lista. Você escreve a carta uma vez, e o JeffPub faz uma cópia para cada pessoa, com o nome e o endereço dela no lugar certo. Uma mesclagem de catálogo repete um mesmo layout para cada item de uma lista.

As ferramentas estão na guia **Correspondências**. Para um caminho guiado, clique em **Correspondências > Iniciar > Mala Direta > Assistente de Mala Direta Passo a Passo** para abrir o painel **Mala Direta**.

## Escolher os destinatários

Clique em **Correspondências > Iniciar > Selecionar Destinatários**:

- **Usar uma Lista Existente…**: escolha um arquivo CSV, um arquivo de texto separado por tabulações, uma planilha .xlsx (a primeira planilha) ou um arquivo de contatos .vcf. Em um arquivo CSV, de texto ou de planilha, a primeira linha deve conter os nomes das colunas.
- **Selecionar dos Contatos…**: escolha um arquivo de contatos (.vcf ou .vcard). A maioria das agendas e dos celulares pode salvar os contatos em um arquivo desse tipo; cada contato se torna um destinatário, com colunas de nome, endereço, telefone e email.
- **Digitar uma Nova Lista…**: preencha a janela **Nova Lista de Endereços**, usando **Nova Entrada** para cada pessoa. Em seguida, o JeffPub oferece salvar a lista como um arquivo CSV.

Para conferir a lista, clique em **Correspondências > Iniciar > Editar Lista de Destinatários**. Na janela **Destinatários da Mala Direta**, desmarque uma linha para pular aquela pessoa, clique em um cabeçalho para classificar e digite em **Filtrar destinatários** para reduzir a lista. **Diferenciar destinatários por** escolhe uma coluna que distingue as pessoas umas das outras. A lista é mantida na publicação com suas linhas desmarcadas e, ao escolher novamente o mesmo arquivo de lista, as mesmas pessoas continuam de fora, mesmo que você tenha classificado as linhas antes. **Localizar Duplicatas** desmarca as repetições, e **Validar Endereços** destaca as linhas com endereço ausente ou CEP estranho.

## Adicionar campos de mesclagem

Um campo de mesclagem é um espaço reservado, como «Nome», que se torna um valor real para cada pessoa. Clique em uma caixa de texto (o JeffPub adiciona uma se você não tiver nenhuma) e use **Correspondências > Gravar e Inserir Campos**:

- **Bloco de Endereço**: um endereço postal montado a partir de colunas com nomes como Nome, Sobrenome e Endereço, linha 1.
- **Linha de Saudação**: "Prezado(a)" e o nome da pessoa, ou "Prezado(a) amigo(a)," se não houver nome.
- **Inserir Campo de Mesclagem**: escolha qualquer coluna.
- **Campo de Imagem**: escolha uma coluna com nomes de arquivos de imagem. O JeffPub adiciona um quadro vazio e procura os arquivos ao lado da sua lista. Depois de salvar a publicação e abri-la novamente, uma imagem cujo arquivo está fora da pasta da publicação não aparece até você escolher novamente a lista de destinatários (**Correspondências > Selecionar Destinatários > Usar uma Lista Existente…**). Então, as imagens da pasta da lista, e das pastas dentro dela, aparecem até você fechar a publicação. O JeffPub não guarda isso para a próxima vez. Ou mantenha os arquivos de imagem na pasta da publicação, ou em uma pasta dentro dela; assim eles aparecem assim que ela é aberta. Isso impede que uma publicação enviada por outra pessoa leia imagens do seu computador por conta própria. A imagem de cada registro aparece quando você visualiza os resultados, imprime, salva um PDF ou mescla para uma nova publicação ou um catálogo.

## Mostrar um número, um preço, uma data ou uma hora do seu jeito

Um campo de mesclagem mostra o valor exatamente como está na sua lista. Para mudar isso, clique ao lado do campo (ou selecione-o, ou selecione vários campos) e clique em **Correspondências > Gravar e Inserir Campos > Formatar Campo de Mesclagem…**. Em **Mostrar o valor como**, escolha:

- **Número**, com de 0 a 4 **Casas decimais**. Por exemplo, 1234,5 com 1 casa decimal aparece como 1.234,5.
- **Moeda**, com o símbolo monetário da sua região: 1234,5 aparece como R$ 1.234,50, onde esse for o símbolo.
- **Porcentagem**, em que 0,256 aparece como 25,6% com 1 casa decimal.
- **Data**, em um **Estilo** como Curto, Longo, Ano, mês, dia, Mês e dia, ou Mês e ano.
- **Hora**, em um **Estilo** Curto, de 12 horas ou de 24 horas.

A linha **Amostra** mostra o resultado da sua escolha. Os valores são lidos e exibidos no idioma do texto no campo (inglês dos Estados Unidos, quando nenhum for definido), não no idioma do seu computador. Em inglês dos Estados Unidos, 03/04/2026 é 4 de março; em um idioma em que o dia vem primeiro, como o alemão ou o inglês britânico, é 3 de abril. Em alemão, 1.234,5 é um número com vírgula decimal. Um ano com dois dígitos é lido como 2000 a 2029 para 00 a 29, e como 1930 a 1999 para 30 a 99. Um número entre parênteses, como (500), é negativo. Um valor que não é um número, uma data ou uma hora do tipo escolhido aparece exatamente como está na sua lista: uma palavra, "nan" ou "inf" (uma pessoa pode se chamar Nan), um valor vazio, uma hora com formato de data, ou uma data com formato de hora. A visualização e a mesclagem final mostram o valor formatado. A escolha conta como uma etapa de **Desfazer** e é salva com a publicação.

## Visualizar os resultados

Clique em **Correspondências > Visualizar Resultados > Visualizar Resultados** para ver dados reais no lugar dos campos. Use **Primeiro Registro**, **Registro Anterior**, **Próximo Registro** e **Último Registro** para percorrer as pessoas, ou **Localizar Destinatário** para ir até uma correspondência.

## Concluir a mesclagem

Clique em **Correspondências > Concluir > Concluir e Mesclar**:

- **Mesclar para Impressora…** abre a página **Imprimir**. Marque **Imprimir todos os registros da mala direta** e imprima.
- **Mesclar em nova publicação** cria uma publicação nova com uma cópia das suas páginas para cada pessoa.
- **Mesclar para PDF…** salva um único PDF com as páginas de todas as pessoas.
- **Mesclar para Email…** cria um arquivo de email .eml por pessoa, com a primeira página como imagem. Escolha a coluna de email, digite um assunto e escolha uma pasta. O JeffPub não envia os emails; abra os arquivos no seu programa de email.
- **Salvar Cada um como Arquivo…** cria uma publicação .jpub para cada pessoa, em uma pasta que você escolher. Nomeie os arquivos com colunas entre chaves, como {Sobrenome}, {Nome}. O JeffPub pede confirmação antes de mais de 200 arquivos e, uma vez, se já existirem arquivos com os mesmos nomes. Clique em **Cancelar** para parar entre um arquivo e outro.
- **Exportar Lista de Destinatários…** salva a lista como um arquivo CSV.

## Mesclagem de catálogo

Clique em **Inserir > Páginas > Páginas de Catálogo** para abrir o painel **Mesclagem de Catálogo**.

1. Escolha a lista de produtos, como descrito acima.
2. Clique em **Inserir Área de Catálogo**. Aparece uma caixa marcada como "Área de catálogo". Defina as **Linhas** e as **Colunas**, além da posição e do tamanho. Coloque os campos de um item na primeira célula, clicando duas vezes neles na lista. Eles se repetem nas outras células. Escolha a versão "(imagem)" de um campo para inserir uma imagem.
3. Visualize com as setas. Cada página mostra tantos itens quanto células a área tiver. Depois, mescle para uma nova publicação, um PDF ou a impressora.

**Remover Área de Catálogo** elimina a área.

## Relacionados

- [Imprimir uma publicação](printing)
- [Adicionar e formatar caixas de texto](text-boxes)
- [A guia Correspondências](tab-mailings)
